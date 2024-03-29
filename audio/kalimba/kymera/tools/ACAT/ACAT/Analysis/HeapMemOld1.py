############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
"""
Heap memory related classes. Implementation before the heap config change.
"""
from ACAT.Analysis.HeapMem import HeapMem as NewHeapMem
from ACAT.Core import CoreTypes as ct

# 'heap_config':() is empty because members are not necessarily accessed,
# 'mem_node' also has members 'line' and 'file' missing since they are in try
VARIABLE_DEPENDENCIES = {
    'strict': ('$_heap_debug_free', '$_heap_debug_min_free'),
    'not_strict': (
        '$_heap1_max_size', '$_heap4_max_size', '$_heap2_max_size',
        '$_heap3_max_size', 'L_pheap_info', '$_heap_info_list', '$_heap1',
        '$_heap2', 'L_freelist1', 'L_freelist2', 'L_freelist3', 'L_freelist4',
        'L_memory_pool_limits'
    )
}
TYPE_DEPENDENCIES = {
    'heap_config': (),
    'mem_node': ('length', 'u', 'u.next', 'u.magic')
}


class HeapMem(NewHeapMem):
    """
    @brief This class encapsulates an analysis for heap memory usage.
    """
    def __init__(self, **kwarg):
        # Call the base class constructor.
        NewHeapMem.__init__(self, **kwarg)
        # Look up the debuginfo once. Don't do it here though; we don't want
        # to throw an exception from the constructor if something goes
        # wrong.
        self._do_debuginfo_lookup = True
        self.pmalloc_debug_enabled = None

        self.freelist1 = None
        self.freelist2 = None
        self.freelist3 = None
        self.freelist4 = None
        self.heap1 = None
        self.heap2 = None
        self.heap3 = None
        self.heap4 = None

    ##################################################
    # Private methods
    ##################################################

    def _check_kymera_version(self):
        """
        @brief This function checks if the Kymera version is compatible with
            this analysis. For outdated Kymera OutdatedFwAnalysisError will be
            raised.
        @param[in] self Pointer to the current object
        """
        pass


    def _get_heap_property(self, heap_number):
        """
        @brief Internal function used to get information about a specific heap.
        @param[in] self Pointer to the current object
        @param[in] heap_number The heap number specifies the heap from which
            information is asked

        @param[out] Tuple containing information about heap.
            (available, heap_size, heap_start, heap_end, heap_free_start)
                available - True, if the heap is present in the build.
                heap_size - Size in octets
                heap_start - Start address
                heap_end - The last valid address
                heap_free_start - The address of the first available block.
        """
        # the old heap only supports 4 types of heap.
        if heap_number > 3:
            return (False, 0, 0, 0, 0)

        freelist = self.__getattribute__("freelist%d" % (heap_number + 1))
        if freelist is None:
            return (False, 0, 0, 0, 0)

        processor_number = self.chipdata.processor
        heap_var = self.__getattribute__("heap%d" % (heap_number + 1))
        try:
            heap_size = self.heap_info_list.members[processor_number].get_member(
                'heap%d' % (heap_number + 1)
            ).get_member('heap_size').value
        except ct.DebugInfoNoVariable:
            heap_size = heap_var.size
        heap_start = self.heap_info_list.members[processor_number].get_member(
            'heap%d' % (heap_number + 1)
        ).get_member('heap_start').value
        heap_end = heap_start + heap_size - 1
        heap_free_start = self.chipdata.get_data(freelist)

        return (True, heap_size, heap_start, heap_end, heap_free_start)


    def _get_heap_config(self, processor_number, heap_number):
        """
        @brief In dual core configuration information about the heap can be read
            for the other processor too.
        @param[in] self Pointer to the current object
        @param[in] processor_number The processor where the heap lives.
        @param[in] heap_number The heap number specifies the heap from which
            information is asked

        @param[out] Tuple containing information about heap.
            (available, heap_size, heap_start, heap_end)
                available - True, if the heap is present in the build.
                heap_size - Size in octets
                heap_start - Start address
                heap_end - The last valid address
        """
        # the old heap only supports 4 types of heap.
        if heap_number > 3:
            return (False, 0, 0, 0)

        available = not self.__getattribute__("freelist%d" % (heap_number + 1)) is None

        heap_size = self.heap_info_list.members[processor_number].get_member(
            'heap%d' % (heap_number + 1)
        ).get_member('heap_size').value
        heap_start = self.heap_info_list.members[processor_number].get_member(
            'heap%d' % (heap_number + 1)
        ).get_member('heap_start').value
        heap_end = heap_start + heap_size - 1

        return (available, heap_size, heap_start, heap_end)

    def _lookup_debuginfo(self):
        """
        @brief Queries debuginfo for information we will need to get the heap
        memory usage.
        @param[in] self Pointer to the current object
        """

        if not self._do_debuginfo_lookup:
            return

        self._do_debuginfo_lookup = False

        # Freelist
        try:
            self.heap_info_list = self.chipdata.get_var_strict("$_heap_info_list")
        except ct.DebugInfoNoVariable:
            self.heap_info_list = None

        for heap_num in range(self.max_num_heaps):
            try:
                # set self.freelist<num>
                self.__setattr__(
                    "freelist%d" % (heap_num + 1),
                    self.debuginfo.get_var_strict(
                        "L_freelist%d" % (heap_num + 1)
                    ).address
                )
                # set self.heap<num>
                self.__setattr__(
                    "heap%d" % (heap_num + 1),
                    self.debuginfo.get_var_strict(
                        "$_heap%d" % (heap_num + 1)
                    )
                )
            except ct.DebugInfoNoVariable:
                # set the free list to None
                self.__setattr__(
                    "freelist%d" % (heap_num + 1),
                    None
                )

        # Check for PMALLOC_DEBUG
        # If L_memory_pool_limits exists then PMALLOC_DEBUG is enabled
        try:
            self.debuginfo.get_var_strict('L_memory_pool_limits')
            self.pmalloc_debug_enabled = True
        except ct.DebugInfoNoVariable:
            self.pmalloc_debug_enabled = False
