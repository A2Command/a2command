;*********************************************************************
;*                                 *            Movie:               *
;*       ProDOS Hyper-FORMAT       *     ProDOS Hyper-FORMAT II      *
;*                                 *           The Return            *
;*     created by Jerry Hewett     *   Modified by Gary Desrochers   *
;*         copyright  1985         *                                 *
;*     Living Legends Software     *        Tryll Software           *
;*                                 *                                 *
;* A Public Domain disk formatting * A Puplic Domain Disk Formatting *
;* routine for the ProDOS Disk Op- * You know the same as -------|   *
;* erating System.  These routines *<----------------------------|   *
;* can be included within your own * Except it now includes code for *
;* software as long as you give us * the 3.5 Disk and the Ram3 Disk. *
;* credit for developing them.     *  Please don't use this for any  *
;*                                 * SmartPort volumes bigger than   *
;*       Updated on: 23Aug85       * 2.1 Meg. Look under Hyper-FORMAT*
;*                                 * in you local BB Library for the *
;*********************************** original code for Hyper-FORMAT  *
;* Ported to CA65 on 15Sep2013     *                                 *
;* By Payton Byrd                  *      Updated on: 22Dec89        *
;*                                 *                                 *
;*********************************************************************

.import popa, popax
.importzp ptr1, ptr2

.import __FORMAT_LOAD__, _cputs
.export _FORMATTER

.segment "FORMAT"

HOME     =  $FC58                    ;MONITOR CLEAR SCREEN AND HOME CURSOR
DEVCNT   =  $BF31                    ;PRODOS DEVICE COUNT
DEVLIST  =  $BF32                    ;LIST OF DEVICES FOR PRODOS
DEVADR   =  $BF10                    ;GIVEN SLOT THIS IS THE ADDRESS OF DRIVER
MLI      =  $BF00                    ;PRODOS MACHINE LANGUAGE INTERFACE
LAST     =  $BF30                    ;LAST DEVICE ACCESSED BY PRODOS
WAIT     =  $FCA8                    ;DELAY ROUTINE
STEP0     =  $C080                    ;DRIVE STEPPER MOTOR POSITIONS
STEP1     =  $C081                    ;  |      |      |       |
STEP2     =  $C082                    ;  |      |      |       |
STEP4     =  $C084                    ;  |      |      |       |
STEP6     =  $C086                    ;  |      |      |       |
DISKOFF   =  $C088                    ;DRIVE OFF  SOFTSWITCH
DISKON    =  $C089                    ;DRIVE ON   SOFTSWITCH
SELECT    =  $C08A                    ;STARTING OFFSET FOR TARGET DEVICE
DISKRD    =  $C08C                    ;DISK READ  SOFTSWITCH
DISKWR    =  $C08D                    ;DISK WRITE SOFTSWITCH
MODERD    =  $C08E                    ;MODE READ  SOFTSWITCH
MODEWR    =  $C08F                    ;MODE WRITE SOFTSWITCH
BUFFEROFF1  = $0E
BUFFEROFF2  = $0F
BUFFEROFF3  = $10
;**********************************************************
; EQUATES
;**********************************************************


_FORMATTER:
         STA SLOT
		 JSR popax
		 STA ptr2
		 STX ptr2+1
		 JSR popa
		 STA DRV

         LDA SLOT
         TAX
JUMP5:   LDY   DEVCNT                   ;LOAD HOW MANY DEVICES
FLOOP:   LDA   DEVLIST,Y                ; SINCE THIS ISN'T A SEQUENTIAL
         STA   LISTSLOT                 ; LIST THEN MUST GO THROUGH EACH ONE
         AND   #$F0                     ; MUST ALSO STORE WHAT IS THERE FOR LATER
         CMP   SLOT
         BEQ   ITISNUM
         DEY
         BPL   FLOOP
         JMP   NOUNIT                   ;USED TO BE BMI
ITISNUM: NOP
         TXA                            ;MAKE THE SLOT THE INDEXED REGISTER
         LSR   A                        ; FOR GETTING DEVICE DRIVE CONTROLLER
         LSR   A
         LSR   A
         TAY
         LDA   DEVADR,Y                 ;GET IT
         STA   ADDRESS                     ; AND SAVE IT
         LDA   DEVADR+1,Y
         STA   ADDRESS+1
         TAY
         AND   #$F0                     ;NEXT SEE IF IT BEGINS WITH A C0
         CMP   #$C0
         BEQ   YESSMART                 ; AND IF IT DOES IS A VERY SMART DEVICE
         TXA
         CMP   #$B0                     ;IF IT ISN'T SMART TEST FOR /RAM
         BEQ   YESRAM3
         CLC
         ROR   A
         ROR   A
         ROR   A
         ROR   A
         AND   #$07
         ORA   #$C0
         STA   ADDRESS+1                   ;IF IT ISN'T EITHER THEN TREAT IT AS
         JMP   YESSMART1                ; SMART AND SAVE WHAT BANK IT IS IN.

YESRAM3: NOP 
         LDA   ADDRESS+1                   ;IF YOU THINK IT IS A /RAM THEN CHECK
         CMP   #$FF                     ; THE BITS THAT TELL YOU SO.
         BEQ   LOOP7                    ; DOES THE ADDRESS POINTER START WITH FF
         JMP   NOUNIT
LOOP7:    LDA   ADDRESS                     ; AND END WITH 00
         CMP   #$00
         BEQ   LOOP8
         JMP   NOUNIT
LOOP8:    LDA   LISTSLOT
         AND   #$F3
         CMP   #$B3
         BEQ   LOOP9
         JMP   NOUNIT
LOOP9:   JSR   RAM3FORM

JUMP2:    JMP   MEXIT
YESSMART: NOP
         TYA
         AND   #$0F
         ROL   A                        ;MOVE LOWER 4 BITS TO UPPER 4 BITS
         ROL   A
         ROL   A
         ROL   A
         STA   SLOT                     ;STORE RESULT IN FORMAT SLOT
YESSMART1: NOP
         LDA   ADDRESS+1                   ;CHECK SIGNATURE BYTES IN THE CN PAGE
         STA   ptr1+1                 ; FOR A SMART DEVICE.
         LDA   #$00
         STA   ptr1
         LDY   #$01
         LDA   (ptr1),Y
         CMP   #$20
         BNE   NOUNIT
         LDY   #$03
         LDA   (ptr1),Y
         BNE   NOUNIT
         LDY   #$05
         LDA   (ptr1),Y
         CMP   #$03
         BNE   NOUNIT
         LDY   #$FF
         LDA   (ptr1),Y
         CMP   #$00                     ;APPLES DISKII
         BEQ   DISKII
         CMP   #$FF                     ;WRONG DISKII
         BEQ   NOUNIT                   ; MUST BE A SMART DEVICE.
         LDY   #$07                     ;TEST LAST SIGNATURE BYTE FOR THE
         LDA   (ptr1),Y               ; PROTOCOL CONVERTER.
         CMP   #$3C
         BEQ   YESSMART2                ; Found a hard drive, CFFA, etc.
         CMP   #$55
         BEQ   YESSMART2                ; Found a ProFile
         CMP   #$00
         BNE   NOUNIT                   ;IT ISN'T SO ITS NO DEVICE I KNOW.
YESSMART2:
         JSR   LNAME                    ;GET NEW NAME
         JSR   SMARTFORM                ;JUMP TO ROUTINE TO FORMAT SMART DRIVE
         LDA   LISTSLOT
         AND   #$F0
         STA   SLOT
         JSR   CODEWR                   ;JUMP TO ROUTINE TO PRODUCE BIT MAP
         JMP   CATALOG                  ;WRITE DIRECTORY INFORMATION TO THE DISK
JUMP3:    JMP   MEXIT


NOUNIT:  LDA   #$4E                     ;THERE IS NO UNIT NUMBER LIKE THAT
         JMP   MEXIT

DISKII:  NOP 
         LDA   #$18                     ;SET VOLBLKS TO 280 ($118)
         LDX   #$01                     ; JUST SETTING DEFAULT SETTINGS
         LDY   #$00
         STA   VOLBLKS
         STX   VOLBLKS+1
         STY   VOLBLKS+2
         JSR   LNAME                    ;GET NEW NAME
         JMP   BEGINFORMAT                  ;FORMAT DISKII

LNAME:   LDY   #$00
WLOOP:   LDA   (ptr2),Y                  ;TRANSFER 'BLANK' TO VOLNAM
         AND   #$7F                     ;CLEAR MSB
         STA   VOLNAM,Y
         INY
         TAX                     ;END OF TRANSFER?
         BNE   WLOOP
LSETLEN:
         CLC
         TYA                            ;ADD $F0 TO VOLUME NAME LENGTH
         ADC   #$F0                     ;CREATE STORAGE_TYPE, NAME_LENGTH BYTE
         STA   VOLLEN
         RTS

BEGINFORMAT:
		 JSR   FORMAT                   ;FORMAT THE DISK
         JSR   CODEWR                   ;FORM BITMAP
         JMP   CATALOG                  ;WRITE DIRECTORY INFORMATION TO THE DISK

CODEWR:  NOP 
         LDA   #$81                     ;SET OPCODE TO WRITE
         STA   OPCODE
;**********************************
;*                                *
;* WRITE BLOCK0 TO DISK           *
;*                                *
;**********************************
ASKBLK0: 
         LDA   #<BOOTCODE                ;SET MLIBUF TO BOOTCODE
         LDY   #>BOOTCODE
         STA   MLIBUF
         STY   MLIBUF+1
         LDA   #$00                     ;SET MLIBLK TO 0
         STA   MLIBLK
         STA   MLIBLK+1
         JSR   CALLMLI                  ;WRITE BLOCK #0 TO TARGET DISK
;**************************************
;* FILL ptr1 $6800-$69FF WITH ZEROS *
;* AND PREPARE BITMAP AND LINK BLOCKS *
;* FOR WRITING TO DISK                *
;*                                    *
;**************************************
FILL:    NOP 
         LDA   #$05                     ;BLOCK 5 ON DISK
         STA   MLIBLK
         LDA   #$00                     ;SET ptr1, MLIBUF TO $6800
         LDX   #(>__FORMAT_LOAD__+ BUFFEROFF2)
         STA   MLIBUF
         STA   ptr1
         STX   MLIBUF+1
         STX   ptr1+1
         TAY                            ;FILL $6800-$69FF WITH ZEROS
         LDX   #$01                     ;FILL 2 PAGES OF 256 BYTES
LZERO:    STA   (ptr1),Y
         INY
         BNE   LZERO
         INC   ptr1+1
         DEX
         BPL   LZERO
         LDA   #(>__FORMAT_LOAD__+ BUFFEROFF2)                     ;RESET ptr1 TO $6800
         STA   ptr1+1
         LDA   #$05                     ;LENGTH OF DIRTBL
         STA   COUNT
LLINK:    LDX   COUNT
         LDA   DIRTBL,X                 ;MOVE DIRECTORY LINK VALUES INTO ptr1
         STA   __FORMAT_LOAD__ + (BUFFEROFF2 * $100) + 2                   ;STORE NEXT DIRECTORY BLOCK #
         DEX
         LDA   DIRTBL,X                 ;FETCH ANOTHER # FROM DIRTBL
         STA   __FORMAT_LOAD__ + (BUFFEROFF2 * $100)                    ;STORE PREVIOUS DIRECTORY BLOCK #
         DEX
         STX   COUNT
         JSR   CALLMLI                  ;WRITE DIRECTORY LINK VALUES TO DISK
LDEC:     DEC   MLIBLK                   ;DECREMENT MLI BLOCK NUMBER
         LDA   MLIBLK                   ;SEE IF MLIBLK = 2
         CMP   #$02
         BNE   LLINK                    ;PROCESS ANOTHER LINK BLOCK
;**********************************
;*                                *
;* CALCULATE BITMAP SIZE AND CNDO *
;*                                *
;**********************************
BLKCOUNT: LDA   VOLBLKS+1                ;WHERE # OF BLOCKS ARE STORED
         LDX   VOLBLKS
         LDY   VOLBLKS+2                ;CAN'T DEAL WITH BLOCK DEVICES THIS BIG
         BNE   JUMP10
         STX   COUNT+1                  ;DEVIDE THE # OF BLOCKS BY 8 FOR BITMAP
         LSR   A                        ; CALCULATION
         ROR   COUNT+1
         LSR   A
         ROR   COUNT+1
         LSR   A
         ROR   COUNT+1
         STA   COUNT+2
         JMP   BITMAPCODE
JUMP10:   PLA                            ;REMOVE THE ADDRESS THAT THE ROUTINE
         PLA                            ; WOULD HAVE RETURNED TO
         LDA   #$4D                     ;MAKE ERROR BLOCK SIZE TO LARGE
         JMP   MEXIT
BITMAPCODE: NOP
         LDA   #%00000001               ;CLEAR FIRST 7 BLOCKS
         STA   (ptr1),Y
         LDY   COUNT+1                  ;ORIGINAL LOW BLOCK COUNT VALUE
         BNE   JUMP11                   ;IF IT IS 0 THEN MAKE FF
         DEY                            ;MAKE FF
         DEC   COUNT+2                  ;MAKE 256 BLOCKS LESS ONE
         STY   COUNT+1                  ;MAKE FF NEW LOW BLOCK VALUE
JUMP11:  NOP 
         LDX   COUNT+2                  ;HIGH BLOCK VALUE
         BNE   JUMP15                   ;IF IT ISN'T EQUAL TO 0 THEN BRANCH
         LDY   COUNT+1
         JMP   JUMP19

JUMP15:  NOP 
         LDA   #(>__FORMAT_LOAD__ + BUFFEROFF3)                      ;SET THE ADRESS OF THE UPPER PART OF
         STA   ptr1+1                 ; BLOCK IN BITMAP BEING CREATED
         LDA   #%11111111
         LDY   COUNT+1                  ;USING THE LOW BYTE COUNT
JUMP20:   DEY
         STA   (ptr1),Y               ;STORE THEM
         BEQ   JUMP17
         JMP   JUMP20
JUMP17:  NOP 
         DEY                            ;FILL IN FIRST PART OF BLOCK
         LDA   #(>__FORMAT_LOAD__+ BUFFEROFF2) 
         STA   ptr1+1
JUMP19:   LDA   #%11111111
         DEY
         STA   (ptr1),Y
         CPY   #$01                     ;EXCEPT THE FIRST BYTE.
         BEQ   JUMP18
         JMP   JUMP19
JUMP18: 
         RTS

;*************************************
;*                                   *
;* CATALOG - BUILD A DIRECTORY TRACK *
;*                                   *
;*************************************
CATALOG:  NOP
         LDA   #$81                     ;CHANGE OPCODE TO $81 (WRITE)
         STA   OPCODE
         LDA   #$00                     ;RESET MLIBUF TO $6800
         LDX   #(>__FORMAT_LOAD__+ BUFFEROFF2) 
         STA   MLIBUF
         STX   MLIBUF+1
         LDX   #$06                     ;WRITE ptr1 (BITMAP) TO BLOCK #6 ON THE DISK
         STX   MLIBLK
         STA   MLIBLK+1
         JSR   CALL2MLI                  ; CALL FOR TIME AND DATE
         JSR   MLI
         .BYTE $82
         .WORD $0000
         LDA   $BF90                    ;GET THEM AND SAVE THEM INTO THE
         STA   DATIME                   ; DIRECTORY TO BE WRITTEN.
         LDA   $BF91
         STA   DATIME+1
         LDA   $BF92
         STA   DATIME+2
         LDA   $BF93
         STA   DATIME+3

         ldy #$2B                       ; Fill area with zero
         lda #$00
:        sta (ptr1), y
         iny
         bne :-


         LDY   #$2A                     ;MOVE BLOCK2 INFORMATION TO $6800
CLOOP:    LDA   BLOCK2,Y
         STA   (ptr1),Y
         DEY
         BPL   CLOOP
         LDA   #$02                     ;WRITE BLOCK #2 TO THE DISK
         STA   MLIBLK
         JSR   CALL2MLI
        
MEXIT:
         RTS
CALL2MLI: NOP
         JSR   CALLMLI
         RTS

;*************************************
;*                                   *
;* CALLMLI - CALL THE MLI READ/WRITE *
;* ROUTINES TO TRANSFER BLOCKS TO OR *
;* FROM MEMORY                       *
;*                                   *
;*************************************
  
CALLMLI: CLC
         JSR   MLI                      ;CALL THE PRODOS MACHINE LANGAUGE INTERFACE
OPCODE:  .BYTE $81                  ;DEFAULT MLI OPCODE = $81 (WRITE)
         .addr PARMS
         BCS   ERROR
         RTS
ERROR:   NOP                            ;(THIS WILL BE CHANGED TO RTS BY VERIFY)
         TAX
         PLA
         PLA
         PLA
         PLA
		 TXA
         JMP   MEXIT

;***********************************
;*                                 *
;* FORMAT - FORMAT THE TARGET DISK *
;*                                 *
;***********************************
FORMAT:
         LDA   SLOT                     ;FETCH TARGET DRIVE SLOTNUM VALUE
         PHA                            ;STORE IT ON THE STACK
         AND   #$70                     ;MASK OFF BIT 7 AND THE LOWER 4 BITS
         STA   SLOTF                    ;STORE RESULT IN FORMAT SLOT STORAGE
         TAX                            ;ASSUME VALUE OF $60 (DRIVE #1)
         PLA                            ;RETRIEVE VALUE FROM THE STACK
         BPL   LDRIVE1                  ;IF < $80 THE DISK IS IN DRIVE #1
         INX                            ;SET X OFFSET TO $61 (DRIVE #2)
LDRIVE1:  LDA   SELECT,X                 ;SET SOFTSWITCH FOR PROPER DRIVE
         LDX   SLOTF                    ;SET X OFFSET TO FORMAT SLOT/DRIVE
         LDA   DISKON,X                 ;TURN THE DRIVE ON
         LDA   MODERD,X                 ;SET MODE SOFTSWITCH TO READ
         LDA   DISKRD,X                 ;READ A BYTE
         LDA   #$23                     ;ASSUME HEAD IS ON TRACK 35
         STA   TRKCUR
         LDA   #$00                     ;DESTINATION IS TRACK 0
         STA   TRKDES
         JSR   SEEK                     ;MOVE HEAD TO TRACK 0
         LDX   SLOTF                    ;TURN OFF ALL DRIVE PHASES
         LDA   STEP0,X
         LDA   STEP2,X
         LDA   STEP4,X
         LDA   STEP6,X
         LDA   TRKBEG                   ;MOVE TRKBEG VALUE (0) TO TRACK
         STA   TRACK
         JSR   BUILD                    ;BUILD A TRACK IN MEMORY AT $6700

;*******************************
;*                             *
;* WRITE - WRITE TRACK TO DISK *
;*                             *
;*******************************
WRITE:    
         JSR   CALC                     ;CALCULATE NEW TRACK/SECTOR/CHECKSUM VALUES
         JSR   TRANS                    ;TRANSFER TRACK IN MEMORY TO DISK
         BCS   DIEDII                   ;IF CARRY SET, SOMETHING DIED
MINC:     INC   TRACK                    ;ADD 1 TO TRACK VALUE
         LDA   TRACK                    ;IS TRACK > ENDING TRACK # (TRKEND)?
         CMP   TRKEND
         BEQ   LNEXT                    ;MORE TRACKS TO FORMAT
         BCS   DONE                     ;FINISHED.  EXIT FORMAT ROUTINE
LNEXT:    STA   TRKDES                   ;MOVE NEXT TRACK TO FORMAT TO TRKDES
         JSR   SEEK                     ;MOVE HEAD TO THAT TRACK
         JMP   WRITE                    ;WRITE ANOTHER TRACK
DONE:     LDX   SLOTF                    ;TURN THE DRIVE OFF
         LDA   DISKOFF,X
         RTS                            ;FORMAT IS FINISHED. RETURN TO CALLING ROUTINE
DIEDII:   PHA                            ;SAVE MLI ERROR CODE ON THE STACK
         JSR   DONE
         PLA                          ;RETRIEVE ERROR CODE FROM THE STACK
         JMP   MEXIT                  ;RETURN ERROR CODE

;************************************
;*                                  *
;* TRANS - TRANSFER TRACK IN MEMORY *
;* TO TARGET DEVICE                 *
;*                                  *
;************************************

TRANS:    NOP
         LDA   #$00                     ;SET ptr1 TO $6700
         LDX   #(>__FORMAT_LOAD__ + BUFFEROFF1)
         STA   ptr1
         STX   ptr1+1
         LDY   #$32                     ;SET Y OFFSET TO 1ST SYNC BYTE (MAX=50)
         LDX   SLOTF                    ;SET X OFFSET TO FORMAT SLOT/DRIVE
         SEC                            ;(ASSUM THE DISK IS WRITE PROTECTED)
         LDA   DISKWR,X                 ;WRITE SOMETHING TO THE DISK
         LDA   MODERD,X                 ;RESET MODE SOFTSWITCH TO READ
         BMI   LWRPROT                  ;IF > $7F THEN DISK WAS WRITE PROTECTED
         LDA   #$FF                     ;WRITE A SYNC BYTE TO THE DISK
         STA   MODEWR,X
         CMP   DISKRD,X
         NOP                            ;(KILL SOME TIME FOR WRITE SYNC...)
         JMP   LSYNC2
LSYNC1:   EOR   #$80                     ;SET MSB, CONVERTING $7F TO $FF (SYNC BYTE)
         NOP                            ;(KILL TIME...)
         NOP
         JMP   MSTORE
LSYNC2:   PHA                            ;(KILL MORE TIME... [ SHEESH! ])
         PLA
LSYNC3:  LDA   (ptr1),Y               ;FETCH BYTE TO WRITE TO DISK
         CMP   #$80                     ;IS IT A SYNC BYTE? ($7F)
         BCC   LSYNC1                   ;YEP. TURN IT INTO AN $FF
         NOP
MSTORE:   STA   DISKWR,X                 ;WRITE BYTE TO THE DISK
         CMP   DISKRD,X                 ;SET READ SOFTSWITCH
         INY                            ;INCREMENT Y OFFSET
         BNE   LSYNC2
         INC   ptr1+1                 ;INCREMENT ptr1 BY 255
         LDA   ptr1 + 1
         CMP    #(>__FORMAT_LOAD__ + BUFFEROFF1 + $1A)
         BNE   LSYNC3                   ;IF < $8000 GET MORE FORMAT DATA
         LDA   MODERD,X                 ;RESTORE MODE SOFTSWITCH TO READ
         LDA   DISKRD,X                 ;RESTORE READ SOFTSWITCH TO READ
         CLC
         RTS
LWRPROT:  CLC                            ;DISK IS WRITE PROTECTED! (NERD!)
         JSR   DONE                     ;TURN THE DRIVE OFF
         PLA
         PLA
         PLA
         PLA
         LDA   #$2B
         JMP   MEXIT                     ;RETURN ERROR CODE
;************************************
;*                                  *
;* BUILD - BUILD GAP1 AND 16 SECTOR *
;* IMAGES BETWEEN $6700 AND $8000   *
;*                                  *
;************************************
BUILD:   NOP
         LDA   #$10                     ;SET ptr1 TO $6710
         LDX   #(>__FORMAT_LOAD__ + BUFFEROFF1)
         STA   ptr1
         STX   ptr1+1
         LDY   #$00                     ;(Y OFFSET ALWAYS ZERO)
         LDX   #$F0                     ;BUILD GAP1 USING $7F (SYNC BYTE)
         LDA   #$7F
         STA   LBYTE
         JSR   LFILL                    ;STORE SYNC BYTES FROM $6710 TO $6800
         LDA   #$10                     ;SET COUNT FOR 16 LOOPS
         STA   COUNT
LIMAGE:   LDX   #$00                     ;BUILD A SECTOR IMAGE IN THE ptr1 AREA
ELOOP:    LDA   LADDR,X                  ;STORE ADDRESS HEADER, INFO & SYNC BYTES
         BEQ   LINFO
         STA   (ptr1),Y
         JSR   LINC                     ;ADD 1 TO ptr1 OFFSET ADDRESS
         INX
         BNE   ELOOP
LINFO:    LDX   #$AB                     ;MOVE 343 BYTES INTO DATA AREA
         LDA   #$96                     ;(4&4 ENCODED VERSION OF HEX $00)
         STA   LBYTE
         JSR   LFILL
         LDX   #$AC
         JSR   LFILL
         LDX   #$00
YLOOP:    LDA   LDATA,X                  ;STORE DATA TRAILER AND GAP3 SYNC BYTES
         BEQ   LDECCNT
         STA   (ptr1),Y
         JSR   LINC
         INX
         BNE   YLOOP
LDECCNT:  CLC
         DEC   COUNT
         BNE   LIMAGE
         RTS                            ;RETURN TO WRITE TRACK TO DISK (WRITE)
LFILL:    LDA   LBYTE
         STA   (ptr1),Y               ;MOVE A REGISTER TO ptr1 AREA
         JSR   LINC                     ;ADD 1 TO ptr1 OFFSET ADDRESS
         DEX
         BNE   LFILL
         RTS
LINC:     CLC
         INC   ptr1                   ;ADD 1 TO ptr1 ADDRESS VECTOR
         BNE   LDONE
         INC   ptr1+1
LDONE:    RTS
;***********************************
;*                                 *
;* CALC - CALCULATE TRACK, SECTOR, *
;* AND CHECKSUM VALUES OF THE NEXT *
;* TRACK USING 4&4 ENCODING        *
;*                                 *
;***********************************
CALC:    NOP 
         LDA   #$03                     ;SET ptr1 TO $6803
         LDX   #(>__FORMAT_LOAD__+ BUFFEROFF2) 
         STA   ptr1
         STX   ptr1+1
         LDA   #$00                     ;SET SECTOR TO 0
         STA   SECTOR
ZLOOP:    LDY   #$00                     ;RESET Y OFFSET TO 0
         LDA   #$FE                     ;SET VOLUME # TO 254 IN 4&4 ENCODING
         JSR   LENCODE
         LDA   TRACK                    ;SET TRACK, SECTOR TO 4&4 ENCODING
         JSR   LENCODE
         LDA   SECTOR
         JSR   LENCODE
         LDA   #$FE                     ;CALCULATE THE CHECKSUM USING 254
         EOR   TRACK
         EOR   SECTOR
         JSR   LENCODE
         CLC                            ;ADD 385 ($181) TO ptr1 ADDRESS
         LDA   ptr1
         ADC   #$81
         STA   ptr1
         LDA   ptr1+1
         ADC   #$01
         STA   ptr1+1
         INC   SECTOR                   ;ADD 1 TO SECTOR VALUE
         LDA   SECTOR                   ;IF SECTOR > 16 THEN QUIT
         CMP   #$10
         BCC   ZLOOP
         RTS                            ;RETURN TO WRITE TRACK TO DISK (WRITE)
LENCODE:  PHA                            ;PUT VALUE ON THE STACK
         LSR   A                        ;SHIFT EVERYTHING RIGHT ONE BIT
         ORA   #$AA                     ;OR IT WITH $AA
         STA   (ptr1),Y               ;STORE 4&4 RESULT IN ptr1 AREA
         INY
         PLA                            ;RETRIEVE VALUE FROM THE STACK
         ORA   #$AA                     ;OR IT WITH $AA
         STA   (ptr1),Y               ;STORE 4&4 RESULT IN ptr1 AREA
         INY
         RTS
;*************************************
;*                                   *
;* SEEK - MOVE HEAD TO DESIRED TRACK *
;*                                   *
;*************************************
SEEK:    NOP 
         LDA   #$00                     ;SET INOUT FLAG TO 0
         STA   LINOUT
         LDA   TRKCUR                   ;FETCH CURRENT TRACK VALUE
         SEC
         SBC   TRKDES                   ;SUBTRACT DESTINATION TRACK VALUE
         BEQ   LEXIT                    ;IF = 0 WE'RE DONE
         BCS   LMOVE
         EOR   #$FF                     ;CONVERT RESULTING VALUE TO A POSITIVE NUMBER
         ADC   #$01
LMOVE:    STA   COUNT                    ;STORE TRACK VALUE IN COUNT
         ROL   LINOUT                   ;CONDITION INOUT FLAG
         LSR   TRKCUR                   ;IS TRACK # ODD OR EVEN?
         ROL   LINOUT                   ;STORE RESULT IN INOUT
         ASL   LINOUT                   ;SHIFT LEFT FOR .TABLE OFFSET
         LDY   LINOUT
ALOOP:    LDA   LTABLE,Y                 ;FETCH MOTOR PHASE TO TURN ON
		 JSR   PHASE                    ;TURN ON STEPPER MOTOR
         LDA   LTABLE+1,Y               ;FETCH NEXT PHASE
         JSR   PHASE                    ;TURN ON STEPPER MOTOR
         TYA
         EOR   #$02                     ;ADJUST Y OFFSET INTO LTABLE
         TAY
         DEC   COUNT                    ;SUBTRACT 1 FROM TRACK COUNT
         BNE   ALOOP
         LDA   TRKDES                   ;MOVE CURRENT TRACK LOCATION TO TRKCUR
         STA   TRKCUR
LEXIT:    RTS                            ;RETURN TO CALLING ROUTINE
;**********************************
;*                                *
;* PHASE - TURN THE STEPPER MOTOR *
;* ON AND OFF TO MOVE THE HEAD    *
;*                                *
;**********************************

PHASE:   NOP 
         ORA   SLOTF                    ;OR SLOT VALUE TO PHASE
         TAX
         LDA   STEP1,X                  ;PHASE ON...
		 LDA   $C012                    ;CHECK IF ROM IS ACTIVE
         BPL   :+
		 LDA   $C08A                    ;IF IT ISN'T, READ ROM NO WRITE B1
:	     LDA   #$56                     ;20 MS. DELAY
         JSR   WAIT
         LDA   STEP0,X                  ;PHASE OFF...
         RTS
;**********************************
;*                                *
;* FORMAT A RAM3 DEVICE.          *
;*                                *
;**********************************
RAM3FORM: NOP
         PHP
         SEI
         LDA   #3                       ;FORMAT REQUEST NUMBER
         STA   $42
         LDA   SLOT                     ;SLOT OF /RAM
         STA   $43
         LDA   #$00                     ;ptr1 SPACE IF NEEDED LOW BYTE
         STA   $44
         LDA   #(>__FORMAT_LOAD__ + BUFFEROFF1)                     ; AND HIGH BYTE
         STA   $45

         LDA   $C08B                    ;READ AND WRITE RAM, USING BANK 1
         LDA   $C08B

         JSR   RAM3DRI
         BIT   $C082                    ;READ ROM, USE BANK 2(PUT BACK ON LINE)
         BCS   RAM3ERR
         PLP
         RTS

RAM3DRI:  JMP   (ADDRESS)
RAM3ERR:  NOP
         TAX
         PLP
         PLA
         PLA
         TXA
         JMP   MEXIT
;**********************************
;*                                *
;* FORMAT A SMARTPORT DEVICE      *
;*                                *
;**********************************
SMARTFORM:
         lda #$03
         sta SMARTCNT
         lda DRV
         sta SMARTUNT
         LDA #$00
         sta SMARTPR1
         lda #(>__FORMAT_LOAD__ + BUFFEROFF2)
         sta SMARTPR1 + 1

		 JSR SMARTDRI

         LDA #$03
         STA   SMARTCMD
         lda #$01
         sta SMARTCNT
         lda DRV
         sec
         sbc #$30
         sta SMARTUNT
         JSR   SMARTDRI
         BCS   SMARTERR
		 CLC
         RTS

SMARTDRI:
		JMP (ADDRESS)
SMARTCMD: .byte $00
SMARTCNT: .byte $00
SMARTUNT: .byte $00
SMARTPR1: .word $0000

SMARTERR:
         TAX
         PLA
         PLA
         TXA
         JMP   MEXIT


;*************************
;*                       *
;* VARIABLE STORAGE AREA *
;*                       *
;*************************
        NOP
        NOP
        NOP
        NOP
        NOP
DRV:        .BYTE $00
INFO:     
         .BYTE $02
         .BYTE $00
         .WORD VOLLEN
PARMS:    .BYTE $03                  ;PARAMETER COUNT = 3
SLOT:     .BYTE $60                  ;DEFAULT TO S6,D1
MLIBUF:   .WORD BOOTCODE             ;DEFAULT ptr1 ADDRESS
MLIBLK:   .WORD $0000                ;DEFAULT BLOCK NUMBER OF 0
QSLOT:    .BYTE $00                        ;QUIT SLOT NUMBER
LISTSLOT: .BYTE $00                        ;SAVING THE SLOT TOTAL FROM THE LIST
ADDRESS:  .WORD $0000
NETPARMS: .BYTE $00
         .BYTE  $2F                     ;COMMAND FOR FILISTSESSIONS
         .WORD  $0000                   ;APPLETALK RESULT CODE RETURNED HERE
         .WORD  $100                    ;LENGTH OF STRING
         .WORD  (__FORMAT_LOAD__ + (BUFFEROFF1 * $100))                   ;ptr1 LOW WORD
         .WORD  $0000                   ;ptr1 HIGH WORD
NETDEV:   .BYTE $00                  ;NUMBER OF ENTRIES RETURNED HERE
LBYTE:    .BYTE $00                        ;STORAGE FOR BYTE VALUE USED IN FILL
LADDR:    .BYTE $D5, $AA, $96              ;ADDRESS HEADER
         .BYTE $AA, $AA, $AA, $AA, $AA, $AA, $AA, $AA                ;VOLUME #, TRACK, SECTOR, CHECKSUM
         .BYTE $DE, $AA, $EB              ;ADDRESS TRAILER
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F                 ;GAP2 SYNC BYTES
         .BYTE $D5, $AA, $AD              ;ptr1 HEADER
         .BYTE $00                    ;END OF ADDRESS INFORMATION
LDATA:   .BYTE $DE, $AA, $EB              ;DATA TRAILER
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F, $7F, $7F                ;GAP3 SYNC BYTES
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F, $7F, $7F                ;GAP3 SYNC BYTES
         .BYTE $00                    ;END OF DATA INFORMATION

LINOUT:   .BYTE $00                        ;INWARD/OUTWARD PHASE FOR STEPPER MOTOR
LTABLE:   .BYTE $02, $04, $06, $00              ;PHASES FOR MOVING HEAD INWARD
         .BYTE $06, $04, $02, $00              ;   |    |    |      |  OUTWARD

WRITING:    .byte $0D, $0A
            .byte "WRITING DATA..."
            .byte $00
FORMATTING: .byte $0D, $0A
            .byte "FORMATTING..."
            .byte $00
FORMATTINGSP: .byte $0D, $0A
            .byte "FORMATTING SMARTPORT..."
            .byte $00
FORMATCOMPLETE: 
            .byte "COMPLETE"
            .byte $0D, $0A, $00
BLANK:   .BYTE "BLANK"
         .ASCIIZ "__________"
VERIF:    .BYTE $0D, $0A, $0D, $0A
		 .BYTE "CERTIFY DISK AND MARK ANY BAD BLOCKS IN ", $0D, $0A
         .ASCIIZ "THE VOLUME BITMAP AS UNUSABLE? (Y/N): "
BAD:      .BYTE $0D, $0A, $0D, $0A
			.ASCIIZ " BAD BLOCK(S) MARKED"
GOOD:     .BYTE $0D, $0A, $0D, $0A
			.ASCIIZ "DISK IS ERROR-FREE"
ITSAII:   .BYTE $0D, $0A
			.ASCIIZ "THIS IS A DISK II"
ITISSMART: .BYTE $0D, $0A
			.BYTE "THIS IS A SMARTPORT DEVICE", $0D, $0A
         .BYTE "CONTINUE WITH FORMAT? (Y/N): ", $00
BLOCK2:   .BYTE $00, $00, $03, $00
VOLLEN:   .BYTE $00                        ;$F0 + LENGTH OF VOLUME NAME
VOLNAM:   .BYTE $00, $00, $00, $00, $00, $00, $00, $00 
		  .BYTE $00, $00, $00, $00, $00, $00, $00		;VOLUME NAME, RESERVED, CREATION, VERSION
RESERVED: .BYTE $00, $00, $32, $1b, $27, $17
UPLOWCASE: .BYTE $00, $84
DATIME:   .BYTE $00, $00, $00, $00
VERSION:  .BYTE $00
         ;.BYTE $00, $C3, $27, $0D
         .BYTE $00, $E3, $27, $0D
         .BYTE $00, $00, $06, $00
VOLBLKS:  .BYTE $00, $00, $00                        ;NUMBER OF BLOCKS AVAILABLE
DIRTBL:   .BYTE $02, $04, $03              ;LINK LIST FOR DIRECTORY BLOCKS
         .BYTE $05, $04, $00
BITTBL:   .BYTE %01111111              ;BITMAP MASK FOR BAD BLOCKS
         .BYTE %10111111
         .BYTE %11011111
         .BYTE %11101111
         .BYTE %11110111
         .BYTE %11111011
         .BYTE %11111101
         .BYTE %11111110
STACK:    .BYTE $00                        ;ENTRANCE STACK POINTER
COUNT:    .BYTE $00, $00, $00                        ;GENERAL PURPOSE COUNTER/STORAGE BYTE
POINTER:  .BYTE $00, $00                        ;STORAGE FOR TRACK COUNT (8 BLOCKS/TRACK)
TRACK:    .BYTE $00, $00                        ;TRACK NUMBER BEING FORMATTED
SECTOR:   .BYTE $00, $00                        ;CURRENT SECTOR NUMBER (MAX=16)
SLOTF:    .BYTE $00, $00                        ;SLOT/DRIVE OF DEVICE TO FORMAT
TRKCUR:   .BYTE $00, $00                        ;CURRENT TRACK POSITION
TRKDES:   .BYTE $00, $00                        ;DESTINATION TRACK POSITION
TRKBEG:   .BYTE $00                   ;STARTING TRACK NUMBER
TRKEND:   .BYTE $23                   ;ENDING TRACK NUMBER
BOOTCODE:
         .byte $01,$38,$B0,$03,$4C,$32,$A1,$86,$43,$C9,$03,$08,$8A,$29,$70,$4A,$4A,$4A,$4A,$09,$C0,$85,$49,$A0
         .byte $FF,$84,$48,$28,$C8,$B1,$48,$D0,$3A,$B0,$0E,$A9,$03,$8D,$00,$08,$E6,$3D,$A5,$49,$48,$A9,$5B,$48
         .byte $60,$85,$40,$85,$48,$A0,$63,$B1,$48,$99,$94,$09,$C8,$C0,$EB,$D0,$F6,$A2,$06,$BC,$1D,$09,$BD,$24
         .byte $09,$99,$F2,$09,$BD,$2B,$09,$9D,$7F,$0A,$CA,$10,$EE,$A9,$09,$85,$49,$A9,$86,$A0,$00,$C9,$F9,$B0
         .byte $2F,$85,$48,$84,$60,$84,$4A,$84,$4C,$84,$4E,$84,$47,$C8,$84,$42,$C8,$84,$46,$A9,$0C,$85,$61,$85
         .byte $4B,$20,$12,$09,$B0,$68,$E6,$61,$E6,$61,$E6,$46,$A5,$46,$C9,$06,$90,$EF,$AD,$00,$0C,$0D,$01,$0C
         .byte $D0,$6D,$A9,$04,$D0,$02,$A5,$4A,$18,$6D,$23,$0C,$A8,$90,$0D,$E6,$4B,$A5,$4B,$4A,$B0,$06,$C9,$0A
         .byte $F0,$55,$A0,$04,$84,$4A,$AD,$02,$09,$29,$0F,$A8,$B1,$4A,$D9,$02,$09,$D0,$DB,$88,$10,$F6,$29,$F0
         .byte $C9,$20,$D0,$3B,$A0,$10,$B1,$4A,$C9,$FF,$D0,$33,$C8,$B1,$4A,$85,$46,$C8,$B1,$4A,$85,$47,$A9,$00
         .byte $85,$4A,$A0,$1E,$84,$4B,$84,$61,$C8,$84,$4D,$20,$12,$09,$B0,$17,$E6,$61,$E6,$61,$A4,$4E,$E6,$4E
         .byte $B1,$4A,$85,$46,$B1,$4C,$85,$47,$11,$4A,$D0,$E7,$4C,$00,$20,$4C,$3F,$09,$26,$50,$52,$4F,$44,$4F
         .byte $53,$20,$20,$20,$20,$20,$20,$20,$20,$20,$A5,$60,$85,$44,$A5,$61,$85,$45,$6C,$48,$00,$08,$1E,$24
         .byte $3F,$45,$47,$76,$F4,$D7,$D1,$B6,$4B,$B4,$AC,$A6,$2B,$18,$60,$4C,$BC,$09,$A9,$9F,$48,$A9,$FF,$48
         .byte $A9,$01,$A2,$00,$4C,$79,$F4,$20,$58,$FC,$A0,$1C,$B9,$50,$09,$99,$AE,$05,$88,$10,$F7,$4C,$4D,$09
         .byte $AA,$AA,$AA,$A0,$D5,$CE,$C1,$C2,$CC,$C5,$A0,$D4,$CF,$A0,$CC,$CF,$C1,$C4,$A0,$D0,$D2,$CF,$C4,$CF
         .byte $D3,$A0,$AA,$AA,$AA,$A5,$53,$29,$03,$2A,$05,$2B,$AA,$BD,$80,$C0,$A9,$2C,$A2,$11,$CA,$D0,$FD,$E9
         .byte $01,$D0,$F7,$A6,$2B,$60,$A5,$46,$29,$07,$C9,$04,$29,$03,$08,$0A,$28,$2A,$85,$3D,$A5,$47,$4A,$A5
         .byte $46,$6A,$4A,$4A,$85,$41,$0A,$85,$51,$A5,$45,$85,$27,$A6,$2B,$BD,$89,$C0,$20,$BC,$09,$E6,$27,$E6
         .byte $3D,$E6,$3D,$B0,$03,$20,$BC,$09,$BC,$88,$C0,$60,$A5,$40,$0A,$85,$53,$A9,$00,$85,$54,$A5,$53,$85
         .byte $50,$38,$E5,$51,$F0,$14,$B0,$04,$E6,$53,$90,$02,$C6,$53,$38,$20,$6D,$09,$A5,$50,$18,$20,$6F,$09
         .byte $D0,$E3,$A0,$7F,$84,$52,$08,$28,$38,$C6,$52,$F0,$CE,$18,$08,$88,$F0,$F5,$BD,$8C,$C0,$10,$FB,$00
         .byte $00,$00,$00,$00,$00,$00,$00,$00

