# Copyright 2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
# or its licensors, as applicable.
#
# You may not use this file except under the terms of the accompanying license.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Project: OCRopus - the open source document analysis and OCR system
# File: Makefile.am
# Purpose: building OCRopus
# Responsible: kofler
# Reviewer:
# Primary Repository: http://ocropus.googlecode.com/svn/trunk/
# Web Sites: www.iupr.org, www.dfki.de


# first build this (libocropus)
SUBDIRS = .

# the folder where all ocropus headers will be installed
ocropusincludedir=$(includedir)/ocropus

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/ocr-utils -I@iulibheaders@ -I@tessheaders@

AM_LDFLAGS =

AM_CXXFLAGS = $(CXXFLAGS) -Wall -Wno-sign-compare -Wno-write-strings -Wno-deprecated

lib_LIBRARIES = libocropus.a

# the default files to compile into libocropus
libocropus_a_SOURCES =  $(srcdir)/ocr-binarize/ocr-binarize-otsu.cc $(srcdir)/ocr-binarize/ocr-binarize-range.cc $(srcdir)/ocr-binarize/ocr-binarize-sauvola.cc $(srcdir)/ocr-commands/ocr-commands.cc $(srcdir)/ocr-layout/line-info.cc $(srcdir)/ocr-layout/log-reg-data.cc $(srcdir)/ocr-layout/ocr-char-stats.cc $(srcdir)/ocr-layout/ocr-classify-zones.cc $(srcdir)/ocr-layout/ocr-color-encode-layout.cc $(srcdir)/ocr-layout/ocr-ctextline-rast-extended.cc $(srcdir)/ocr-layout/ocr-ctextline-rast.cc $(srcdir)/ocr-layout/ocr-deskew-rast.cc $(srcdir)/ocr-layout/ocr-detect-columns.cc $(srcdir)/ocr-layout/ocr-detect-paragraphs.cc $(srcdir)/ocr-layout/ocr-doc-clean-concomp.cc $(srcdir)/ocr-layout/ocr-doc-clean.cc $(srcdir)/ocr-layout/ocr-extract-gutters.cc $(srcdir)/ocr-layout/ocr-extract-rulings.cc $(srcdir)/ocr-layout/ocr-layout-1cp.cc $(srcdir)/ocr-layout/ocr-layout-rast.cc $(srcdir)/ocr-layout/ocr-layout-smear.cc $(srcdir)/ocr-layout/ocr-noisefilter.cc $(srcdir)/ocr-layout/ocr-pageframe-rast.cc $(srcdir)/ocr-layout/ocr-pageseg-wcuts.cc $(srcdir)/ocr-layout/ocr-pageseg-xycut.cc $(srcdir)/ocr-layout/ocr-reading-order.cc $(srcdir)/ocr-layout/ocr-segmentations.cc $(srcdir)/ocr-layout/ocr-text-image-seg.cc $(srcdir)/ocr-layout/ocr-visualize-layout-rast.cc $(srcdir)/ocr-layout/ocr-whitespace-cover.cc $(srcdir)/ocr-layout/ocr-word-segmentation.cc $(srcdir)/ocr-leptonica/ocr-text-image-seg-leptonica.cc $(srcdir)/ocr-line/glclass.cc $(srcdir)/ocr-line/glcuts.cc $(srcdir)/ocr-line/glfmaps.cc $(srcdir)/ocr-line/glutils.cc $(srcdir)/ocr-line/linerec.cc $(srcdir)/ocr-lineseg/ocr-cseg-ccs.cc $(srcdir)/ocr-lineseg/ocr-cseg-projection.cc $(srcdir)/ocr-lineseg/seg-ccs.cc $(srcdir)/ocr-lineseg/seg-cuts.cc $(srcdir)/ocr-lineseg/seg-eval.cc $(srcdir)/ocr-lineseg/seg-skel.cc $(srcdir)/ocr-pfst/a-star.cc $(srcdir)/ocr-pfst/beam-search.cc $(srcdir)/ocr-pfst/lattice.cc $(srcdir)/ocr-pfst/ocrofst-heap.cc $(srcdir)/ocr-pfst/ocrofst-impl.cc $(srcdir)/ocr-pfst/ocrofst-io.cc $(srcdir)/ocr-pfst/ocrofst-util.cc $(srcdir)/ocr-utils/components.cc $(srcdir)/ocr-utils/didegrade.cc $(srcdir)/ocr-utils/editdist.cc $(srcdir)/ocr-utils/grouper.cc $(srcdir)/ocr-utils/init-ocropus.cc $(srcdir)/ocr-utils/linesegs.cc $(srcdir)/ocr-utils/logger.cc $(srcdir)/ocr-utils/narray-io.cc $(srcdir)/ocr-utils/ocr-utils.cc $(srcdir)/ocr-utils/pagesegs.cc $(srcdir)/ocr-utils/resource-path.cc $(srcdir)/ocr-utils/sysutil.cc $(srcdir)/ocr-utils/xml-entities.cc $(srcdir)/ocr-voronoi/bit_func.cc $(srcdir)/ocr-voronoi/cline.cc $(srcdir)/ocr-voronoi/dinfo.cc $(srcdir)/ocr-voronoi/draw_line.cc $(srcdir)/ocr-voronoi/edgelist.cc $(srcdir)/ocr-voronoi/erase.cc $(srcdir)/ocr-voronoi/geometry.cc $(srcdir)/ocr-voronoi/hash.cc $(srcdir)/ocr-voronoi/heap.cc $(srcdir)/ocr-voronoi/img_to_site.cc $(srcdir)/ocr-voronoi/label_func.cc $(srcdir)/ocr-voronoi/memory.cc $(srcdir)/ocr-voronoi/output.cc $(srcdir)/ocr-voronoi/read_image.cc $(srcdir)/ocr-voronoi/sites.cc $(srcdir)/ocr-voronoi/usage.cc $(srcdir)/ocr-voronoi/voronoi-ocropus.cc $(srcdir)/ocr-voronoi/voronoi-pageseg.cc $(srcdir)/ocr-voronoi/voronoi.cc 

# folders for installing models and words
modeldir=${datadir}/ocropus/models
worddir=${datadir}/ocropus/words

# install the data
model_DATA = $(srcdir)/data/models/*
word_DATA = $(srcdir)/data/words/*


noinst_PROGRAMS = 

if use_gsl
    AM_CPPFLAGS += -DHAVE_GSL
endif

if ! notesseract
    AM_CPPFLAGS += -I@tessheaders@ -DHAVE_TESSERACT
    libocropus_a_SOURCES += $(srcdir)/ocr-tesseract/tesseract.cc
    noinst_PROGRAMS += 

endif

if use_leptonica
    AM_CPPFLAGS += -I@leptheaders@ -DHAVE_LEPTONICA
endif


ocropusinclude_HEADERS =  $(srcdir)/include/glclass.h $(srcdir)/include/glcuts.h $(srcdir)/include/gldataset.h $(srcdir)/include/glfmaps.h $(srcdir)/include/glinerec.h $(srcdir)/include/glutils.h $(srcdir)/include/grouper.h $(srcdir)/include/gsl.h $(srcdir)/include/line-info.h $(srcdir)/include/ocr-layout.h $(srcdir)/include/ocr-openfst.h $(srcdir)/include/ocr-pfst.h $(srcdir)/include/ocropus.h $(srcdir)/include/tesseract.h $(srcdir)/ocr-utils/arraypaint.h $(srcdir)/ocr-utils/components.h $(srcdir)/ocr-utils/didegrade.h $(srcdir)/ocr-utils/docproc.h $(srcdir)/ocr-utils/editdist.h $(srcdir)/ocr-utils/enumerator.h $(srcdir)/ocr-utils/grid.h $(srcdir)/ocr-utils/init-ocropus.h $(srcdir)/ocr-utils/linesegs.h $(srcdir)/ocr-utils/logger.h $(srcdir)/ocr-utils/narray-binio.h $(srcdir)/ocr-utils/narray-io.h $(srcdir)/ocr-utils/ocr-utils.h $(srcdir)/ocr-utils/ocrinterfaces.h $(srcdir)/ocr-utils/pages.h $(srcdir)/ocr-utils/pagesegs.h $(srcdir)/ocr-utils/queue.h $(srcdir)/ocr-utils/resource-path.h $(srcdir)/ocr-utils/segmentation.h $(srcdir)/ocr-utils/stringutil.h $(srcdir)/ocr-utils/sysutil.h $(srcdir)/ocr-utils/xml-entities.h

bin_PROGRAMS =  ocr-distance  ocropus
ocr_distance_SOURCES = $(srcdir)/commands/ocr-distance.cc
ocr_distance_LDADD = libocropus.a
ocropus_SOURCES = $(srcdir)/commands/ocropus.cc
ocropus_LDADD = libocropus.a

noinst_PROGRAMS +=  main-ocr-binarize-otsu  main-ocr-binarize-sauvola  main-be  main-voronoi-ocropus
main_ocr_binarize_otsu_SOURCES = $(srcdir)/ocr-binarize/main-ocr-binarize-otsu.cc
main_ocr_binarize_otsu_LDADD = libocropus.a
main_ocr_binarize_sauvola_SOURCES = $(srcdir)/ocr-binarize/main-ocr-binarize-sauvola.cc
main_ocr_binarize_sauvola_LDADD = libocropus.a
main_be_SOURCES = $(srcdir)/ocr-voronoi/main-be.cc
main_be_LDADD = libocropus.a
main_voronoi_ocropus_SOURCES = $(srcdir)/ocr-voronoi/main-voronoi-ocropus.cc
main_voronoi_ocropus_LDADD = libocropus.a

check_PROGRAMS =  test-binarize-sauvola  test-narray-io  test-ocr-utils  test-seg-cuts
test_binarize_sauvola_SOURCES = $(srcdir)/ocr-utils/tests/test-binarize-sauvola.cc
test_binarize_sauvola_LDADD = libocropus.a
test_binarize_sauvola_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/ocr-utils \
-I@iulibheaders@ -I@colibheaders@ -I@tessheaders@
test_narray_io_SOURCES = $(srcdir)/ocr-utils/tests/test-narray-io.cc
test_narray_io_LDADD = libocropus.a
test_narray_io_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/ocr-utils \
-I@iulibheaders@ -I@colibheaders@ -I@tessheaders@
test_ocr_utils_SOURCES = $(srcdir)/ocr-utils/tests/test-ocr-utils.cc
test_ocr_utils_LDADD = libocropus.a
test_ocr_utils_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/ocr-utils \
-I@iulibheaders@ -I@colibheaders@ -I@tessheaders@
test_seg_cuts_SOURCES = $(srcdir)/ocr-utils/tests/test-seg-cuts.cc
test_seg_cuts_LDADD = libocropus.a
test_seg_cuts_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/ocr-utils \
-I@iulibheaders@ -I@colibheaders@ -I@tessheaders@

check:
	@echo "# running tests"
	$(srcdir)/test-binarize-sauvola $(srcdir)/data/testimages
	$(srcdir)/test-narray-io $(srcdir)/data/testimages
	$(srcdir)/test-ocr-utils $(srcdir)/data/testimages
	$(srcdir)/test-seg-cuts $(srcdir)/data/testimages

# run check-style everytime and give a hint about make check
all:
	$(srcdir)/utilities/check-style -f $(srcdir)
	@echo
	@echo "Use 'make check' to run tests!"
	@echo

