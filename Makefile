CC = clang
CFLAGS = -std=c17 -Wall -Wextra -MMD -MP
LDFLAGS = -lm

TOPDIR = .

VECTORG_C = $(TOPDIR)/vectorg.c
VECTORG_D = $(VECTORG_C:.c=.d)
VECTORG = $(VECTORG_C:.c=)

$(VECTORG): $(VECTORG_C)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

-include $(VECTORG_D)
