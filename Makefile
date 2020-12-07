mmu: mmu.cpp
	g++ -std=c++17 -g mmu.cpp -o mmu 

clean: 
	rm -f mmu *~  