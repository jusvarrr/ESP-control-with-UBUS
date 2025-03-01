LDFAGS := -lubox -lblobmsg_json -lubus -lserialport

SRCS := $(wildcard src/*.c)

OUTPUT := espcontrol

$(OUTPUT): $(SRCS)
	$(CC) -g -o $(OUTPUT) $(SRCS) $(LDFAGS)

clean:
	rm -f $(OUTPUT)
