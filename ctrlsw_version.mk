MAJOR=0
MINOR=8
STEP=0
CTRLSW_VER?=$(shell ./setlocalversion.sh)

VERSION=$(MAJOR).$(MINOR).$(STEP)$(CTRLSW_VER)

