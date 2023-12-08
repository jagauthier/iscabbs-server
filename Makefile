#Compiler and Linker
CC          :=  gcc -std=gnu18

#The Target Binary Program
TARGET      := bbs
#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := .
CNV_SRC_DIR := convert_x86_x64
DATA_SRC_DIR := data_migration_tools
UTIL_SRC_DIR := utils
INCDIR      := inc
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

LDFLAGS  +=
LIBS    += $(SSLLIB) -lcrypt # -lnsl -lsocket
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

#CFLAGS   += -O2
CFLAGS   += -g
CFLAGS   += -Wall
CFLAGS   += -Wextra
#CFLAGS   += -Werror
## These warnings should be cleaned up ASAP one by one,
## they are listed in severity order (most severe is at the top).


SOURCES     := \
	./main.c \
	./backup.c

SETUP_SOURCES := \
	./setupbbs.c

CONVERT_SOURCES := \
	$(CNV_SRC_DIR)/convert_data.c \
	$(CNV_SRC_DIR)/convert_bigbtmp.c \
	$(CNV_SRC_DIR)/convert_msgdata.c \
	$(CNV_SRC_DIR)/convert_user.c \
	$(CNV_SRC_DIR)/convert_messages.c \
	$(CNV_SRC_DIR)/convert_room_descs.c \
	$(CNV_SRC_DIR)/rebuild_messages.c \
	$(CNV_SRC_DIR)/convert_mail.c \
	$(CNV_SRC_DIR)/convert_utils.c \
	$(CNV_SRC_DIR)/rebuild_xmsgs.c \

# For iterative structure updates
DATA_MGMT_SRC := \
	$(DATA_SRC_DIR)/room_shuffle.c


UTIL_SOURCES := \
	$(UTIL_SRC_DIR)/utils_main.c \
	$(UTIL_SRC_DIR)/monitor_bigbtmp.c
	
ALL_BBS_SOURCES := \
	./doc.c \
	./doc_aide.c \
	./doc_msgs.c \
	./doc_rooms.c \
	./doc_routines.c \
	./finger.c \
	./global.c \
	./io.c \
	./qmisc.c \
	./qrunbbs.c \
	./qstate.c \
	./queue.c \
	./searchtool.c \
	./sem.c \
	./setup.c \
	./shell.c \
	./state.c \
	./syncer.c \
	./system.c \
	./sysutil.c \
	./term.c \
	./update.c \
	./user.c \
	./users.c \
	./utility.c \
	./vote.c \
	./who.c \
	./xmsg.c

# Common for all builds
ALL_BBS_OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(ALL_BBS_SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# for the BBS
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# for the conversion, utils, and setup
CONVERT_OBJECTS     := $(patsubst $(CNV_SRC_DIR)/%,$(BUILDDIR)/%,$(CONVERT_SOURCES:.$(SRCEXT)=.$(OBJEXT)))
UTIL_OBJECTS     := $(patsubst $(UTIL_SRC_DIR)/%,$(BUILDDIR)/%,$(UTIL_SOURCES:.$(SRCEXT)=.$(OBJEXT)))
SETUP_OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SETUP_SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# for iterative structure updates
DATA_MGMT_OBJS     := $(patsubst $(DATA_SRC_DIR)/%,$(BUILDDIR)/%,$(DATA_MGMT_SRC:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: directories $(TARGET)

setupbbs: $(SETUP_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

convert: $(CONVERT_OBJECTS) $(ALL_BBS_OBJECTS) 
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

room_analysis: $(ALL_BBS_OBJECTS)  $(BUILDDIR)/room_analysis.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

bbs_utils: $(UTIL_OBJECTS) $(ALL_BBS_OBJECTS) 
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

send_x_cmd: $(ALL_BBS_OBJECTS) $(BUILDDIR)/send_x_cmd.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)


#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Remake
remake: cleaner all

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))
-include $(ALL_BBS_OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(ALL_BBS_OBJECTS) $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LDFLAGS) $(LIBS)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

$(BUILDDIR)/%.$(OBJEXT): $(CNV_SRC_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(CNV_SRC_DIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)

$(BUILDDIR)/%.$(OBJEXT): $(DATA_SRC_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(DATA_SRC_DIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)

$(BUILDDIR)/%.$(OBJEXT): $(UTIL_SRC_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(UTIL_SRC_DIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)

#Non-File Targets
#.PHONY: all remake clean cleaner resources

install: $(TARGET)
	install -m 550 bin/bbs ../bin/bbs
	#install -m 550 ssl/ssl_server ../bin/ssl_server
