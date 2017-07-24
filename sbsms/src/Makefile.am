AM_CXXFLAGS = -funroll-loops -fpascal-strings -ffast-math -I../include -I./vst -fstrict-aliasing $(SNDFILE_CFLAGS) $(PORTAUDIO_CFLAGS) $(WX_CFLAGS) $(VST_CFLAGS)

libsbsmsincludedir = $(includedir)
libsbsmsinclude_HEADERS = ../include/sbsms.h

lib_LTLIBRARIES = libsbsms.la
libsbsms_la_SOURCES = slide.cpp sincCoeffs.h sms.cpp track.cpp trackpoint.cpp utils.cpp resample.cpp sbsms.cpp fft.cpp grain.cpp buffer.cpp subband.cpp buffer.h fft.h utils.h defs.h peak.h trackpoint.h grain.h pitch.h real.h resample.h ../include/sbsms.h sms.h subband.h track.h peak.cpp peak.h

libsbsms_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@

bin_PROGRAMS = sbsmsampler sbsms
EXTRA_PROGRAMS = sbsmsampler sbsms

sbsms_SOURCES = test.cpp mp3.cpp mp3.h mp3tech.cpp mp3tech.h audiobuffer.cpp audiobuffer.h import.cpp import.h pcm.cpp pcm.h convert.cpp convert.h synth.cpp synth.h gen.h gen.cpp filter.h filter.cpp dwgs.hg dwgs.cpp synthTable.h synthTable.cpp


sbsms_LDADD = libsbsms.la $(SNDFILE_LIBS) $(MAD_LIBS)
sbsms_LDFLAGS = $(os_ldflags)

sbsmsampler_LDADD = libsbsms.la $(VST_LIBS) $(SNDFILE_LIBS) $(MAD_LIBS)
sbsmsampler_LDFLAGS = $(os_ldflags) -bundle
sbsmsampler_SOURCES = genctrl.cpp genctrl.h dwgs.cpp dwgs.h filter.cpp filter.h gen.cpp synthTable.cpp synthTable.h synth.cpp synth.h convert.cpp convert.h mp3.cpp mp3.h mp3tech.cpp mp3tech.h import.cpp import.h pcm.cpp pcm.h audiobuffer.cpp audiobuffer.h render.cpp render.h vstsbsms.cpp vstsbsms.h sbsmsampler.cpp sbsmsampler.h midi.cpp midi.h voice.cpp voice.h sample.cpp sample.h vstprogram.cpp vstprogram.h sbsmsamplergui.cpp sbsmsamplergui.h sampleview.cpp sampleview.h programview.cpp programview.h ctabviewx.cpp ctabviewx.h statictext.cpp statictext.h rightclickcontrols.cpp rightclickcontrols.h tempoctrl.cpp tempoctrl.h wavedisplaysliders.cpp wavedisplaysliders.h $(VSTSDK_DIR)/audioeffect.cpp $(VSTSDK_DIR)/audioeffect.h $(VSTSDK_DIR)/audioeffectx.cpp $(VSTSDK_DIR)/audioeffectx.h $(VSTSDK_DIR)/aeffeditor.h $(VSTSDKROOT_DIR)/pluginterfaces/vst2.x/aeffect.h $(VSTSDKROOT_DIR)/pluginterfaces/vst2.x/aeffectx.h $(VSTGUI_DIR)/aeffguieditor.cpp $(VSTGUI_DIR)/aeffguieditor.h $(VSTGUI_DIR)/cfileselector.cpp $(VSTGUI_DIR)/cfileselector.h $(VSTGUI_DIR)/vstcontrols.cpp $(VSTGUI_DIR)/vstcontrols.h $(VSTGUI_DIR)/vstgui.cpp $(VSTGUI_DIR)/vstgui.h $(VSTSDK_DIR)/vstplugmain.cpp
all: $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst $(top_builddir)/vst/mac/Info.plist $(top_builddir)/vst/mac/PkgInfo

$(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst: sbsmsampler
	mkdir -p $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents/MacOS
	mkdir -p $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents/Resources
	cp -f sbsmsampler $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents/MacOS/sbsmsampler
	cp -f $(top_builddir)/vst/mac/Info.plist $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents
	cp -f $(top_builddir)/vst/images/* $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents/Resources
	cp -f $(top_builddir)/vst/mac/PkgInfo $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst/Contents/PkgInfo
	rm -rf ~/Library/Audio/Plug-Ins/VST/sbsmsampler.vst &&	cp -rf  $(top_builddir)/approot/Library/Audio/Plug-Ins/VST/sbsmsampler.vst ~/Library/Audio/Plug-Ins/VST
