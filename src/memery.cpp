#include "assert.h"
#include <list>

#include "cd2mem.h"
#include "exploit.h"
#include "singly_linked.h"
#include "exploit.h"

using namespace std;

uintptr_t starting_addr;
uintptr_t ending_addr;
char* memblock;

void usage() {
    cout << "This program does first scan analysis of pointers in the heap of the core" << endl
         << "Usage: map_pointers core start_addr end_addr" << endl 
         << " core       : filename of heap core dump" << endl
         << " start_addr : starting heap addr as hexadecimal address" << endl
         << " end_addr   : ending heap address as hexadecimal address" << endl; 
    return;
}

int main(int argc, char *argv[]) {
    if (setup_exploit()) return 1;
	starting_addr = exploit_startaddr();
	ending_addr = exploit_endaddr();

    /* number of and array of qwords in heap memory */
	long num_p = ((HEAP_SIZE*2) / 8);
    assert(num_p == (ending_addr - starting_addr) / 8);
	struct mem_ptr* p_arr = (struct mem_ptr*) malloc(num_p * sizeof(struct mem_ptr));
    read_vuln(0);
    
    init_pointers(p_arr, num_p);

    list<struct mem_struct*>* ds_list = find_singly_linked_ds(p_arr, num_p);

	// print out entire ds(s) starting from root(s)
	for (auto i: *(ds_list)) {
        finalize_nodes(p_arr, i);
        pretty_print_struct(p_arr, i);
	}

    // TODO: Doubly-linked lists
	// TODO: Detect function pointers?!

    /* Meeting 12-2-2019 */
    // TODO: Write non-consecutive test (Nathan)
            // Doubly linked should run algorithm on existing singly linked bookkeeping
    // TODO: Detect invariants 

    cout << endl;
    return 0;
}

