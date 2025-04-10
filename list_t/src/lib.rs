#![feature(ptr_as_ref_unchecked)]
use std::{
    alloc::{alloc_zeroed, dealloc, handle_alloc_error, Layout},
    ffi::c_void,
    fmt::{Debug, Display},
    process::exit,
    ptr::{null_mut, NonNull}, slice::from_raw_parts_mut,
};

// typedef struct {
//     void **data ;
//     size_t capacity ;
//     size_t length ;
// } list_t ;

#[repr(C)]
#[derive(Clone)]
#[allow(non_camel_case_types)]
pub struct list {
    data: NonNull<*mut c_void>,
    capacity: usize,
    length: usize,
}

#[allow(non_camel_case_types)]
pub type list_t = *mut list;

impl Default for list {
    fn default() -> Self {
        unsafe {
            list {
                data: NonNull::new_unchecked(null_mut()),
                capacity: 0,
                length: 0,
            }
        }
    }
}

impl Debug for list {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("List")
            .field("Addr", &self.data)
            .field("Len", &self.length)
            .field("Cap", &self.capacity)
            .finish()
    }
}

impl Display for list {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.length == 0 {
            return write!(f, "[]");
        }
        let addrs = (0..self.length).map(|i| unsafe { self.data.add(i).read() as u64 });
        f.debug_list().entries(addrs).finish()
    }
}

impl list {
    #[inline]
    fn sort(&mut self, greater_than: extern "C" fn(*mut c_void, *mut c_void) -> u64) {
        let slice: &mut [*mut c_void] = unsafe {
            from_raw_parts_mut(self.data.as_mut(), self.length)
        };

        slice.sort_by(|a, b| {
            match greater_than(*a, *b) {
                0 => std::cmp::Ordering::Greater,
                1 => std::cmp::Ordering::Less,
                _ => exit(1)
            }
        });
    }

    #[inline]
    fn realloc(&mut self) {
        let cvoid_layout = Layout::new::<*mut c_void>();

        let old = self.data;
        let old_cap = self.capacity;

        let new: NonNull<*mut c_void> = unsafe {
            let size = cvoid_layout.size().unchecked_mul(self.capacity * 2);
            let layout = Layout::from_size_align_unchecked(size, cvoid_layout.align());
            // Should probably [`realloc`] here.
            let new = alloc_zeroed(layout);

            if new.is_null() {
                handle_alloc_error(layout);
            }

            NonNull::new(new as *mut *mut c_void).unwrap()
        };

        for i in 0..self.length {
            unsafe {
                new.add(i).write(self.data.add(i).read());
            }
        }

        self.data = new;
        self.capacity *= 2;

        unsafe {
            let size = cvoid_layout.size().unchecked_mul(old_cap);
            let layout = Layout::from_size_align_unchecked(size, cvoid_layout.align());
            // we assume all data stored here will be managed by c.
            dealloc(old.as_ptr() as *mut u8, layout);
        }
    }

    #[inline]
    fn append_item(&mut self, item: *mut c_void) {
        if self.capacity == self.length {
            self.realloc();
        }

        unsafe {
            self.data.add(self.length).write(item);
        }

        self.length += 1;
    }
}

/// _: New (a la Python __init__)
/// inputs: none
/// outputs: a new list_t containing no values
/// side effects: none
#[unsafe(no_mangle)]
pub extern "C" fn list_new() -> list_t {
    let layout = Layout::array::<*mut c_void>(4).unwrap();
    let init = unsafe { alloc_zeroed(layout) };

    if init.is_null() {
        handle_alloc_error(layout);
    }

    let ptr = init as *mut *mut c_void;
    let data = NonNull::new(ptr).unwrap();

    let list_ret = Box::leak(Box::new(list::default()));

    list_ret.data = data;
    list_ret.length = 0;
    list_ret.capacity = 4;

    list_ret
}

/// _: Free (no Python equivalent)
/// inputs: a list_t l
/// outputs: nothing
/// side effects: frees all memory associated with l
#[unsafe(no_mangle)]
pub extern "C" fn list_free(l: list_t) {
    let cvoid_layout = Layout::new::<*mut c_void>();

    unsafe {
        let size = cvoid_layout.size().unchecked_mul((*l).capacity);
        let layout = Layout::from_size_align_unchecked(size, cvoid_layout.align());
        // we assume all data stored here will be managed by c.
        dealloc((*l).data.as_ptr() as *mut u8, layout);
        drop(Box::from_raw(l));
    }
}

/// _: Print (a la Python __str__)
/// _: Print (a la Python __str__)
/// inputs: a list_t l
/// outputs: nothing
/// list_print(list_new()) ;
/// - should print "[]"
/// Note: Prints void *'s as uint64_t's. (use %lu)
#[unsafe(no_mangle)]
pub extern "C" fn list_print(l: list_t) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    println!("{}", list_ref);
}

/// 0: Append
/// inputs: a list_t l, a pointer to an memory object of any type x
/// outputs: nothing
/// side effects: x is added to the end of l
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// list_print(l) ;
/// - should print "[1]"
#[unsafe(no_mangle)]
pub extern "C" fn list_append(l: list_t, x: *mut c_void) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };

    if list_ref.length == list_ref.capacity {
        list_ref.realloc();
    }

    unsafe {
        list_ref.data.add(list_ref.length).write(x);
    }

    list_ref.length += 1;
}

/// 1: Extend
/// inputs: two list_ts, l1 and l2
/// outputs: nothing
/// side effects: all elements of l2 are appended to l1
/// uint64_t *val = 1 ;
/// list_append(l1, val) ;
/// list_t l2 = list_new() ;
/// *val = 2 ;
/// list_append(l2, val) ;
/// list_print(l1) ;
/// - should print "[1, 2]"
#[unsafe(no_mangle)]
pub extern "C" fn list_extend(l1: list_t, l2: list_t) {
    let l1_ref: &mut list = unsafe { l1.as_mut_unchecked() };
    let l2_ref: &mut list = unsafe { l2.as_mut_unchecked() };

    for i in 0..l2_ref.length {
        unsafe {
            l1_ref.append_item(l2_ref.data.add(i).read());
        }
    }
}

/// 2: Insert
/// inputs: a list_t l, size_t list index i, and a pointer to an memory object of any type x
/// outputs: nothing
/// side effects: x is added to l with index i and all elements are preserved in l
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// *val = 2 ;
/// list_insert(l, 0, val) ;
/// list_print(l) ;
/// - should print "[2, 1]"
#[unsafe(no_mangle)]
pub extern "C" fn list_insert(l: list_t, i: usize, x: *mut c_void) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };

    if list_ref.length == list_ref.capacity {
        list_ref.realloc();
    }

    for i in (i..=list_ref.length).rev() {
        unsafe {
            list_ref.data.add(i + 1).write(list_ref.data.add(i).read());
        }
    }

    list_ref.length += 1;

    unsafe {
        list_ref.data.add(i).write(x);
    }
}

/// 3: Remove
/// inputs: a list_t l, and a pointer to an memory object of any type x
/// outputs: TRUE if an instance of x is removed, FALSE otherwise
/// side effects: the first instance of x is removed from l
/// uint64_t *val = 2 ;
/// list_append(l, val) ;
/// list_append(l, val) ;
/// *val = 1 ;
/// list_print(l1) ;
/// list_remove(l, 1, val) ;
/// list_print(l) ;
/// - should return True
/// - should print "[2, 1, 2]" then "[1, 2]"
#[unsafe(no_mangle)]
pub extern "C" fn list_remove(l: list_t, x: *mut c_void) -> u64 {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };

    let mut index = None;

    for i in 0..list_ref.length {
        unsafe {
            if list_ref.data.add(i).read() == x {
                index = Some(i);
                break;
            }
        }
    }

    // Don't wanna make it one loop.
    if let Some(i) = index {
        for i in i..(list_ref.length - 1) {
            unsafe {
                list_ref.data.add(i).write(list_ref.data.add(i + 1).read());
            }
        }
        list_ref.length -= 1;
        return 1;
    } else {
        return 0;
    }
}

/// 4: Pop
/// inputs: a list_t l, and a size_t list index i
/// outputs: The element at index i, or exit(1) if i is out of range
/// side effects: remove the element at index i
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// *val = 2 ;
/// list_insert(l, 0, val) ;
/// list_print(l) ;
/// printf("%d\n", pop(l, 1)) ;
/// list_print(l) ;
/// - should print "[1, 2]" then "2" then "[1]"
#[unsafe(no_mangle)]
pub extern "C" fn list_pop(l: list_t, i: usize) -> *mut c_void {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    if i >= list_ref.length {
        exit(1);
    }

    let item = unsafe { list_ref.data.add(i).read() };

    for i in i..(list_ref.length - 1) {
        unsafe {
            list_ref.data.add(i).write(list_ref.data.add(i + 1).read());
        }
    }
    list_ref.length -= 1;

    item
}

/// 5: Clear
/// inputs: a list_t l
/// outputs: nothing
/// side effects: l contains no elements
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// clear(l)
/// - should print "[]"
#[unsafe(no_mangle)]
pub extern "C" fn list_clear(l: list_t) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    list_ref.length = 0;
}

/// 6: Index
/// inputs: a list_t l, and a pointer to an memory object of any type x
/// outputs: a size_t i giving the index of x in l, or exit(1) if x is not in l.
/// side effects: none
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// printf("%d\n", index(l, val))) ;
/// - should print "0"
#[unsafe(no_mangle)]
pub extern "C" fn list_index(l: list_t, x: *mut c_void) -> usize {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    for i in 0..list_ref.length {
        unsafe {
            if list_ref.data.add(i).read() == x {
                return i;
            }
        }
    }
    exit(1);
}

/// 7: Count
/// inputs: a list_t l, and a pointer to an memory object of any type x
/// outputs: The number of times x occurs in l
/// side effects: none
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// list_append(l, val) ;
/// list_count(l, val) ;
/// - should return 2
#[unsafe(no_mangle)]
pub extern "C" fn list_count(l: list_t, x: *mut c_void) -> u64 {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    let mut count = 0;
    for i in 0..list_ref.length {
        unsafe {
            if list_ref.data.add(i).read() == x {
                count += 1;
            }
        }
    }
    count
}

/// "Extra credit" for a sorting algorithm
/// More "Extra credit" for an O(n*log(n)) sort
/// */
/// /* void sort(list_t l, bool (greater_than)(void *, void *)) */
#[unsafe(no_mangle)]
pub extern "C" fn sort(l: list_t, greater_than: extern "C" fn(*mut c_void, *mut c_void) -> u64) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    list_ref.sort(greater_than);
}

/// inputs: a list_t l
/// outputs: nothing
/// side effects: the elements of l are reversed
/// example:
/// list_append(l, val) ;
/// *val = 2 ;
/// list_append(l, val) ;
/// list_print(l) ;
/// list_reverse(l)
/// list_print(l) ;
/// - should print "[1, 2]" then "[2, 1]"
#[unsafe(no_mangle)]
pub extern "C" fn list_reverse(l: list_t) {
    let list_ref: &mut list = unsafe { l.as_mut_unchecked() };
    for i in 0..(list_ref.length / 2) {
        unsafe {
            let first = list_ref.data.add(i).read();
            let last = list_ref.data.add(list_ref.length - 1 - i).read();
            list_ref.data.add(i).write(last);
            list_ref.data.add(list_ref.length - 1 - i).write(first);
        }
    }
}

/// 9: Copy
/// inputs: a list_t l
/// outputs: a list_t r containing all the elements of l
/// side effects: none
/// uint64_t *val = 1 ;
/// list_append(l, val) ;
/// list_t r = copy(l) ;
/// list_append(l, val) ;
/// list_print(l) ;
/// list_print(r) ;
/// - should print "[1, 1]" then "[1]"
#[unsafe(no_mangle)]
pub extern "C" fn list_copy(l: list_t) -> list_t {
    unsafe {
        let new = (*l).clone();
        Box::leak(Box::new(new))
    }
}
