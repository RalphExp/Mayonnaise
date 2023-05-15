TARGET=mayonnaise

mayonnaise: main.o
	g++ -o$@ $<

main.o: main.cc
	g++ -o$@ -c $^

clean:
	rm -rf *.o
	rm -rf $(TARGET)