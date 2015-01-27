#
# @(#)Makefile	1.5 93/10/07 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=xxxd xxxe xxxf
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=xxxa xxxb xxxc 
CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=
MANPAGE_FILES= \
		Cell.3 \
		CellB.3 \
		faxG3.3 \
		faxG4.3 \
		H261.3 \
		intro.3 \
		Jpeg.3 \
		JpegLL.3 \
		Mpeg1.3 \
		PhotoCD.3 \
		Storage.3 \
		XIL.3 \
		xil.macs \
		xil_absolute.3 \
		xil_add.3 \
		xil_add_const.3 \
		xil_affine.3 \
		xil_and.3 \
		xil_and_const.3 \
		xil_band_combine.3 \
		xil_black_generation.3 \
		xil_blend.3 \
		xil_call_next_error_handler.3 \
		xil_cast.3 \
		xil_choose_colormap.3 \
		xil_cis_attempt_recovery.3 \
		xil_cis_create.3 \
		xil_cis_destroy.3 \
		xil_cis_flush.3 \
		xil_cis_get_attribute.3 \
		xil_cis_get_autorecover.3 \
		xil_cis_get_bits_ptr.3 \
		xil_cis_get_by_name.3 \
		xil_cis_get_compression_type.3 \
		xil_cis_get_compressor.3 \
		xil_cis_get_input_type.3 \
		xil_cis_get_keep_frames.3 \
		xil_cis_get_max_frames.3 \
		xil_cis_get_name.3 \
		xil_cis_get_output_type.3 \
		xil_cis_get_random_access.3 \
		xil_cis_get_read_frame.3 \
		xil_cis_get_read_invalid.3 \
		xil_cis_get_start_frame.3 \
		xil_cis_get_state.3 \
		xil_cis_get_write_frame.3 \
		xil_cis_get_write_invalid.3 \
		xil_cis_has_data.3 \
		xil_cis_has_frame.3 \
		xil_cis_number_of_frames.3 \
		xil_cis_put_bits.3 \
		xil_cis_put_bits_ptr.3 \
		xil_cis_reset.3 \
		xil_cis_seek.3 \
		xil_cis_set_attribute.3 \
		xil_cis_set_autorecover.3 \
		xil_cis_set_keep_frames.3 \
		xil_cis_set_max_frames.3 \
		xil_cis_set_name.3 \
		xil_cis_sync.3 \
		xil_close.3 \
		xil_color_convert.3 \
		xil_color_correct.3 \
		xil_colorcube_create.3 \
		xil_colorspace_create.3 \
		xil_colorspace_destroy.3 \
		xil_colorspace_get_by_name.3 \
		xil_colorspace_get_name.3 \
		xil_colorspace_get_state.3 \
		xil_colorspace_get_type.3 \
		xil_colorspace_set_name.3 \
		xil_colorspacelist_create.3 \
		xil_colorspacelist_destroy.3 \
		xil_colorspacelist_get_by_name.3 \
		xil_colorspacelist_get_name.3 \
		xil_colorspacelist_get_state.3 \
		xil_colorspacelist_set_name.3 \
		xil_compress.3 \
		xil_convolve.3 \
		xil_copy.3 \
		xil_copy_pattern.3 \
		xil_copy_with_planemask.3 \
		xil_create.3 \
		xil_create_child.3 \
		xil_create_copy.3 \
		xil_create_double_buffered_window.3 \
		xil_create_from_device.3 \
		xil_create_from_type.3 \
		xil_create_from_window.3 \
		xil_create_from_special_window.3 \
		xil_create_temporary.3 \
		xil_create_temporary_from_type.3 \
		xil_decompress.3 \
		xil_default_error_handler.3 \
		xil_destroy.3 \
		xil_device_create.3 \
		xil_device_destroy.3 \
		xil_device_set_attribute.3 \
		xil_device_set_value.3 \
		xil_dilate.3 \
		xil_dithermask_create.3 \
		xil_dithermask_create_copy.3 \
		xil_dithermask_destroy.3 \
		xil_dithermask_get_by_name.3 \
		xil_dithermask_get_height.3 \
		xil_dithermask_get_name.3 \
		xil_dithermask_get_nbands.3 \
		xil_dithermask_get_state.3 \
		xil_dithermask_get_values.3 \
		xil_dithermask_get_width.3 \
		xil_dithermask_set_name.3 \
		xil_divide.3 \
		xil_divide_by_const.3 \
		xil_divide_into_const.3 \
		xil_edge_detection.3 \
		xil_erode.3 \
		xil_error_diffusion.3 \
		xil_error_get_category.3 \
		xil_error_get_category_string.3 \
		xil_error_get_id.3 \
		xil_error_get_location.3 \
		xil_error_get_object.3 \
		xil_error_get_primary.3 \
		xil_error_get_string.3 \
		xil_export.3 \
		xil_extrema.3 \
		xil_fill.3 \
		xil_get_active_buffer.3 \
		xil_get_attribute.3 \
		xil_get_by_name.3 \
		xil_get_child_offsets.3 \
		xil_get_datatype.3 \
		xil_get_device_attribute.3 \
		xil_get_exported.3 \
		xil_get_height.3 \
		xil_get_imagetype.3 \
		xil_get_info.3 \
		xil_get_memory_storage.3 \
		xil_get_name.3 \
		xil_get_nbands.3 \
		xil_get_origin.3 \
		xil_get_origin_x.3 \
		xil_get_origin_y.3 \
		xil_get_parent.3 \
		xil_get_pixel.3 \
		xil_get_readable.3 \
		xil_get_roi.3 \
		xil_get_size.3 \
		xil_get_state.3 \
		xil_get_storage_movement.3 \
		xil_get_storage_with_copy.3 \
		xil_get_synchronize.3 \
		xil_get_tile_storage.3 \
		xil_get_tilesize.3 \
		xil_get_width.3 \
		xil_get_writable.3 \
		xil_get_writeable.3 \
		xil_histogram.3 \
		xil_histogram_create.3 \
		xil_histogram_create_copy.3 \
		xil_histogram_destroy.3 \
		xil_histogram_get_by_name.3 \
		xil_histogram_get_info.3 \
		xil_histogram_get_limits.3 \
		xil_histogram_get_name.3 \
		xil_histogram_get_nbands.3 \
		xil_histogram_get_nbins.3 \
		xil_histogram_get_state.3 \
		xil_histogram_get_values.3 \
		xil_histogram_set_name.3 \
		xil_imagetype_get_by_name.3 \
		xil_imagetype_get_datatype.3 \
		xil_imagetype_get_height.3 \
		xil_imagetype_get_info.3 \
		xil_imagetype_get_name.3 \
		xil_imagetype_get_nbands.3 \
		xil_imagetype_get_size.3 \
		xil_imagetype_get_state.3 \
		xil_imagetype_get_width.3 \
		xil_imagetype_set_name.3 \
		xil_import.3 \
		xil_install_error_handler.3 \
		xil_interpolation_table_create.3 \
		xil_interpolation_table_create_copy.3 \
		xil_interpolation_table_destroy.3 \
		xil_interpolation_table_get_data.3 \
		xil_interpolation_table_get_kernel_size.3 \
		xil_interpolation_table_get_subsamples.3 \
		xil_interpolation_table_get_values.3 \
		xil_kernel_create.3 \
		xil_kernel_create_copy.3 \
		xil_kernel_create_separable.3 \
		xil_kernel_destroy.3 \
		xil_kernel_get_by_name.3 \
		xil_kernel_get_height.3 \
		xil_kernel_get_key_x.3 \
		xil_kernel_get_key_y.3 \
		xil_kernel_get_name.3 \
		xil_kernel_get_state.3 \
		xil_kernel_get_values.3 \
		xil_kernel_get_width.3 \
		xil_kernel_set_name.3 \
		xil_lookup.3 \
		xil_lookup_convert.3 \
		xil_lookup_create.3 \
		xil_lookup_create_combined.3 \
		xil_lookup_create_copy.3 \
		xil_lookup_destroy.3 \
		xil_lookup_get_band_lookup.3 \
		xil_lookup_get_by_name.3 \
		xil_lookup_get_colorcube.3 \
		xil_lookup_get_colorcube_info.3 \
		xil_lookup_get_input_datatype.3 \
		xil_lookup_get_input_nbands.3 \
		xil_lookup_get_name.3 \
		xil_lookup_get_num_entries.3 \
		xil_lookup_get_offset.3 \
		xil_lookup_get_output_datatype.3 \
		xil_lookup_get_output_nbands.3 \
		xil_lookup_get_state.3 \
		xil_lookup_get_values.3 \
		xil_lookup_get_version.3 \
		xil_lookup_set_name.3 \
		xil_lookup_set_offset.3 \
		xil_lookup_set_values.3 \
		xil_max.3 \
		xil_min.3 \
		xil_multiply.3 \
		xil_multiply_const.3 \
		xil_nearest_color.3 \
		xil_not.3 \
		xil_object_get_error_string.3 \
		xil_object_get_type.3 \
		xil_open.3 \
		xil_or.3 \
		xil_or_const.3 \
		xil_ordered_dither.3 \
		xil_paint.3 \
		xil_remove_error_handler.3 \
		xil_rescale.3 \
		xil_roi_add_image.3 \
		xil_roi_add_rect.3 \
		xil_roi_add_region.3 \
		xil_roi_create.3 \
		xil_roi_create_copy.3 \
		xil_roi_destroy.3 \
		xil_roi_get_as_image.3 \
		xil_roi_get_as_region.3 \
		xil_roi_get_by_name.3 \
		xil_roi_get_name.3 \
		xil_roi_get_state.3 \
		xil_roi_intersect.3 \
		xil_roi_set_name.3 \
		xil_roi_subtract_rect.3 \
		xil_roi_translate.3 \
		xil_roi_unite.3 \
		xil_rotate.3 \
		xil_scale.3 \
		xil_sel_create.3 \
		xil_sel_create_copy.3 \
		xil_sel_destroy.3 \
		xil_sel_get_by_name.3 \
		xil_sel_get_height.3 \
		xil_sel_get_key_x.3 \
		xil_sel_get_key_y.3 \
		xil_sel_get_name.3 \
		xil_sel_get_state.3 \
		xil_sel_get_values.3 \
		xil_sel_get_width.3 \
		xil_sel_set_name.3 \
		xil_set_active_buffer.3 \
		xil_set_attribute.3 \
		xil_set_colorspace.3 \
		xil_set_data_supply_routine.3 \
		xil_set_device_attribute.3 \
		xil_set_memory_storage.3 \
		xil_set_name.3 \
		xil_set_origin.3 \
		xil_set_pixel.3 \
		xil_set_roi.3 \
		xil_set_storage_movement.3 \
		xil_set_storage_with_copy.3 \
		xil_set_synchronize.3 \
		xil_set_tile_storage.3 \
		xil_set_tilesize.3 \
		xil_set_value.3 \
		xil_soft_fill.3 \
		xil_squeeze_range.3 \
		xil_state_get_default_tilesize.3 \
		xil_state_get_default_tiling_mode.3 \
		xil_state_get_interpolation_tables.3 \
		xil_state_get_show_action.3 \
		xil_state_get_synchronize.3 \
		xil_state_set_default_tilesize.3 \
		xil_state_set_default_tiling_mode.3 \
		xil_state_set_interpolation_tables.3 \
		xil_state_set_show_action.3 \
		xil_state_set_synchronize.3 \
		xil_storage_create.3 \
		xil_storage_destroy.3 \
		xil_storage_get_band_stride.3 \
		xil_storage_get_by_name.3 \
		xil_storage_get_coordinates.3 \
		xil_storage_get_data.3 \
		xil_storage_get_image.3 \
		xil_storage_get_name.3 \
		xil_storage_get_offset.3 \
		xil_storage_get_pixel_stride.3 \
		xil_storage_get_scanline_stride.3 \
		xil_storage_get_state.3 \
		xil_storage_is_type.3 \
		xil_storage_set_band_stride.3 \
		xil_storage_set_coordinates.3 \
		xil_storage_set_data.3 \
		xil_storage_set_data_release.3 \
		xil_storage_set_name.3 \
		xil_storage_set_offset.3 \
		xil_storage_set_pixel_stride.3 \
		xil_storage_set_scanline_stride.3 \
		xil_subsample_adaptive.3 \
		xil_subsample_binary_to_gray.3 \
		xil_subtract.3 \
		xil_subtract_const.3 \
		xil_subtract_from_const.3 \
		xil_swap_buffers.3 \
		xil_sync.3 \
		xil_tablewarp.3 \
		xil_tablewarp_horizontal.3 \
		xil_tablewarp_vertical.3 \
		xil_threshold.3 \
		xil_toss.3 \
		xil_translate.3 \
		xil_transpose.3 \
		xil_xor.3 \
		xil_xor_const.3

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(MANPAGE_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

PGMS=${ARCH}/xxx-your-program-here ${ARCH}/xxx-another-one

#
# (OPTIONAL) Add local stuff here using +=
#

DEFINES  +=
CPPFLAGS +=
CFLAGS   +=

LD_PATHS  += xxx-your-EXTRA-library-paths-go-here, if any
LD_LIBS   += xxx-your-libraries-go-here eg: -lvx
LDFLAGS   += 

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS += xxx-your-debris-goes-here
CLOBBER_ADDITIONS +=

all:	$(MANPAGE_FILES)

# Install the programs, appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(INSTALL) -m 444 $(MANPAGE_FILES) ${PROJECT_RELEASE}/man/man3
	-$(CATMAN) -w -M $(PROJECT_RELEASE)/man 3

include $(PROJECT_ROOT)/make/rules/leaf.cfg
