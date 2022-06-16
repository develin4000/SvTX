#->====================================<-
#->= SvTX - © Copyright 2022 OnyxSoft =<-
#->====================================<-
#->= Version  : 1.0                   =<-
#->= File     : Makefile              =<-
#->= Author   : Stefan Blixth         =<-
#->= Compiled : 2022-06-16            =<-
#->====================================<-

#
# Application specific stuff
#

APPNAME				= SvTX

APP_MORPHOS			= $(APPNAME)
APP_MORPHOS_DB		= $(APPNAME)_db

#
# Compiler tools assign
#
CC_MORPHOS			= gcc
STRIP_MORPHOS		= strip


#
# Platform specific compiler and linker flags 
#
CFLG_MOS			= -noixemul -D__AMIGADATE__=\"$(shell date "+%d.%m.%y")\" -O2
LFLG_MOS			=
OPTS_MOS			=
OBJS_MOS				= $(APPNAME)_morphos.o

CFLG_MOS_DB			= -noixemul -D__AMIGADATE__=\"$(shell date "+%d.%m.%y")\" -DUSEDEBUG
LFLG_MOS_DB			= -ldebug
OPTS_MOS_DB			=
OBJS_MOS_DB			= $(APPNAME)_morphos_db.o

#
# Source & include files
#
SOURCE_APP			= main.c LinkedList.c rendermcc.c urlparser.c base64.c

.PHONY:	clean usage


usage:
	@echo ""
	@echo "  Application - $(APPNAME)"
	@echo " +-------------------------------------------------------------------+"
	@echo " | clean       - Deletes all files in the obj & release-directories. |"
	@echo " | all         - Make all of the options below                       |"
	@echo " +-------------------------------------------------------------------+"
	@echo " | morphos     - Compiles a binary for MorphOS.                      |"
	@echo " | morphos_db  - Compiles a debug-enabled binary for MorphOS.        |"
	@echo " +-------------------------------------------------------------------+"
	@echo ""

all:	morphos morphos_db

clean:
	@echo "Cleaning up..."
	@echo ""
	rm -f $(APP_MORPHOS)
	rm -f $(APP_MORPHOS_DB)
	@echo ""
	@echo "Done."


morphos_db: $(APP_MORPHOS_DB)
	@echo ""
	@echo "Debug-enabled MorphOS binary sucessfully built..."
	@echo ""

$(APP_MORPHOS_DB):	$(SOURCE_APP)
	$(CC_MORPHOS) $(SOURCE_APP) $(CFLG_MOS_DB) $(OPTS_MOS_DB) $(LFLG_MOS_DB) -o $(APP_MORPHOS_DB)


morphos: $(APP_MORPHOS)
	@echo ""
	@echo "MorphOS binary sucessfully built..."
	@echo ""

$(APP_MORPHOS):	$(SOURCE_APP)
	$(CC_MORPHOS) $(SOURCE_APP) $(OPTS_MOS) $(CFLG_MOS) -o $(APP_MORPHOS)
#	$(STRIP_MORPHOS) $(APP_MORPHOS)
