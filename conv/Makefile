SRCDIR	:= ./src
OBJDIR	:= ./obj
TARGET	:= bin/converter
SRCS	:= $(wildcard $(SRCDIR)/*.c)
OBJS	:= ${subst $(SRCDIR), $(OBJDIR), $(SRCS:.c=.o)}
CCFLAGS := -Wall -Werror -pedantic -std=c89
LDFLAGS := -lm
CC      := gcc

all: release

debug: CCFLAGS += -g
debug: prebuild executable

release: CCFLAGS += -O3
release: prebuild executable

prebuild:
	@echo "Setup..."
	@echo "	 mkdir -p $(OBJDIR)"; mkdir -p $(OBJDIR)
	@echo "Compiling..."
	@echo $(SRCS)

executable: $(OBJS)
	@echo "Linking..."
	@echo "	 $(CC) $^ -o $(TARGET) $(LDFLAGS)"; $(CC) $^ -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "	 $(CC) $< -c -o $@ $(CCFLAGS)"; $(CC) $< -c -o $@ $(CCFLAGS)
	
clean:
	@echo "Cleaning...";
	@echo "	 $(RM) -r $(OBJDIR)"; $(RM) -r $(OBJDIR)
	@echo "	 $(RM) $(TARGET)"; $(RM) $(TARGET)