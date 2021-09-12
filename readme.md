load c library
https://www.ics.uci.edu/~aburtsev/238P/hw/hw3-elf/hw3-elf.html
ELF format
http://www.skyfree.org/linux/references/ELF_Format.pdf
https://blog.k3170makan.com/2018/09/introduction-to-elf-file-format-part.html
http://blog.k3170makan.com/2018/10/introduction-to-elf-format-part-vi.html
https://refspecs.linuxfoundation.org/elf/gabi4+/

load c obj(elf) dynamically

calling function from main to lib and back

global data relocation(missing)
The global data entry is not mapped to the virtual address
How?
need to perform relocation (using .o file before linking)