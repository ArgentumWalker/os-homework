#include <stdint.h>
#include "desc.h"
#include "IDT.h"
#include "IDT_entry.h"
#include "memory.h"

#define i16 __uint128_t 
#define TYPE_F (uint64_t)0x0

i16 idt_table[256];
uint64_t entry_pointer[100];
uint32_t PART = -1;

void entry_pointers_ini();

void IDT_init() {
    for (int i = 0; i < 100; i++) {
        idt_table[i] = 0;
        idt_table[i] = ((i16)(entry_pointer[i] >> 32) << 64) | ((i16)(((uint64_t)1 << (7+8)) | (TYPE_F << 6)) << 48) | ((i16)(entry_pointer[i] & PART) << 16) | ((i16)(KERNEL_CS));
    }
    struct desc_table_ptr ptr = {(uint64_t)(idt_table), 100};
    write_idtr(&ptr);
}


#define makeEntry(no) entry_pointer[no] = entry##no;

void entry_pointers_ini() {
    makeEntry(0);
    makeEntry(1);
    makeEntry(2);
    makeEntry(3);
    makeEntry(4);
    makeEntry(5);
    makeEntry(6);
    makeEntry(7);
    makeEntry(8);
    makeEntry(9);
    makeEntry(10);
    makeEntry(11);
    makeEntry(12);
    makeEntry(13);
    makeEntry(14);
    makeEntry(15);
    makeEntry(16);
    makeEntry(17);
    makeEntry(18);
    makeEntry(19);
    makeEntry(20);
    makeEntry(21);
    makeEntry(22);
    makeEntry(23);
    makeEntry(24);
    makeEntry(25);
    makeEntry(26);
    makeEntry(27);
    makeEntry(28);
    makeEntry(29);
    makeEntry(30);
    makeEntry(31);
    makeEntry(32);
    makeEntry(33);
    makeEntry(34);
    makeEntry(35);
    makeEntry(36);
    makeEntry(37);
    makeEntry(38);
    makeEntry(39);
    makeEntry(40);
    makeEntry(41);
    makeEntry(42);
    makeEntry(43);
    makeEntry(44);
    makeEntry(45);
    makeEntry(46);
    makeEntry(47);
    makeEntry(48);
    makeEntry(49);
    makeEntry(50);
    makeEntry(51);
    makeEntry(52);
    makeEntry(53);
    makeEntry(54);
    makeEntry(55);
    makeEntry(56);
    makeEntry(57);
    makeEntry(58);
    makeEntry(59);
    makeEntry(60);
    makeEntry(61);
    makeEntry(62);
    makeEntry(63);
    makeEntry(64);
    makeEntry(65);
    makeEntry(66);
    makeEntry(67);
    makeEntry(68);
    makeEntry(69);
    makeEntry(70);
    makeEntry(71);
    makeEntry(72);
    makeEntry(73);
    makeEntry(74);
    makeEntry(75);
    makeEntry(76);
    makeEntry(77);
    makeEntry(78);
    makeEntry(79);
    makeEntry(80);
    makeEntry(81);
    makeEntry(82);
    makeEntry(83);
    makeEntry(84);
    makeEntry(85);
    makeEntry(86);
    makeEntry(87);
    makeEntry(88);
    makeEntry(89);
    makeEntry(90);
    makeEntry(91);
    makeEntry(92);
    makeEntry(93);
    makeEntry(94);
    makeEntry(95);
    makeEntry(96);
    makeEntry(97);
    makeEntry(98);
    makeEntry(99);
}

