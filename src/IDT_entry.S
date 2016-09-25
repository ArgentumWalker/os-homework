.global tbl

IDT_S_HANDLER:
    //some_push
    movq %rsp, %rdi;
    call idt_c_handler;
    //some pop
    add $16, %rsp;
    iretq    


#define Exception(no)         \
        entry##no:            \
            push $(no);       \
            jmp IDT_S_HANDLER;\

#define Interrupt(no)         \
        entry##no:            \
            push $(0);        \
            push $(no);       \
            jmp IDT_S_HANDLER;\
        
        
Interrupt(0)
Interrupt(1)
Interrupt(2)
Interrupt(3)
Interrupt(4)
Interrupt(5)
Interrupt(6)
Interrupt(7)
Exception(8)
Interrupt(9)
Exception(10)
Exception(11)
Exception(12)
Exception(13)
Exception(14)
Interrupt(15)
Interrupt(16)
Exception(17)
Interrupt(18)
Interrupt(19)
Interrupt(20)
Interrupt(21)
Interrupt(22)
Interrupt(23)
Interrupt(24)
Interrupt(25)
Interrupt(26)
Interrupt(27)
Interrupt(28)
Interrupt(29)
Interrupt(30)
Interrupt(31)
Interrupt(32)
Interrupt(33)
Interrupt(34)
Interrupt(35)
Interrupt(36)
Interrupt(37)
Interrupt(38)
Interrupt(39)
Interrupt(40)
Interrupt(41)
Interrupt(42)
Interrupt(43)
Interrupt(44)
Interrupt(45)
Interrupt(46)
Interrupt(47)
Interrupt(48)
Interrupt(49)
Interrupt(50)
Interrupt(51)
Interrupt(52)
Interrupt(53)
Interrupt(54)
Interrupt(55)
Interrupt(56)
Interrupt(57)
Interrupt(58)
Interrupt(59)
Interrupt(60)
Interrupt(61)
Interrupt(62)
Interrupt(63)
Interrupt(64)
Interrupt(65)
Interrupt(66)
Interrupt(67)
Interrupt(68)
Interrupt(69)
Interrupt(70)
Interrupt(71)
Interrupt(72)
Interrupt(73)
Interrupt(74)
Interrupt(75)
Interrupt(76)
Interrupt(77)
Interrupt(78)
Interrupt(79)
Interrupt(80)
Interrupt(81)
Interrupt(82)
Interrupt(83)
Interrupt(84)
Interrupt(85)
Interrupt(86)
Interrupt(87)
Interrupt(88)
Interrupt(89)
Interrupt(90)
Interrupt(91)
Interrupt(92)
Interrupt(93)
Interrupt(94)
Interrupt(95)
Interrupt(96)
Interrupt(97)
Interrupt(98)
Interrupt(99)

tbl:
    .quad entry0
    .quad entry1
    .quad entry2
    .quad entry3
    .quad entry4
    .quad entry5
    .quad entry6
    .quad entry7
    .quad entry8
    .quad entry9
    .quad entry10
    .quad entry11
    .quad entry12
    .quad entry13
    .quad entry14
    .quad entry15
    .quad entry16
    .quad entry17
    .quad entry18
    .quad entry19
    .quad entry20
    .quad entry21
    .quad entry22
    .quad entry23
    .quad entry24
    .quad entry25
    .quad entry26
    .quad entry27
    .quad entry28
    .quad entry29
    .quad entry30
    .quad entry31
    .quad entry32
    .quad entry33
    .quad entry34
    .quad entry35
    .quad entry36
    .quad entry37
    .quad entry38
    .quad entry39
    .quad entry40
    .quad entry41
    .quad entry42
    .quad entry43
    .quad entry44
    .quad entry45
    .quad entry46
    .quad entry47
    .quad entry48
    .quad entry49
    .quad entry50
    .quad entry51
    .quad entry52
    .quad entry53
    .quad entry54
    .quad entry55
    .quad entry56
    .quad entry57
    .quad entry58
    .quad entry59
    .quad entry60
    .quad entry61
    .quad entry62
    .quad entry63
    .quad entry64
    .quad entry65
    .quad entry66
    .quad entry67
    .quad entry68
    .quad entry69
    .quad entry70
    .quad entry71
    .quad entry72
    .quad entry73
    .quad entry74
    .quad entry75
    .quad entry76
    .quad entry77
    .quad entry78
    .quad entry79
    .quad entry80
    .quad entry81
    .quad entry82
    .quad entry83
    .quad entry84
    .quad entry85
    .quad entry86
    .quad entry87
    .quad entry88
    .quad entry89
    .quad entry90
    .quad entry91
    .quad entry92
    .quad entry93
    .quad entry94
    .quad entry95
    .quad entry96
    .quad entry97
    .quad entry98
    .quad entry99
