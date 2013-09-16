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

.import __FORMAT_LOAD__, _cputs, _cgetc, _cputc

.export _FORMATTER, _SLOT, _GETYN

.segment "FORMAT"

HOME     =  $FC58                    ;MONITOR CLEAR SCREEN AND HOME CURSOR
DEVCNT   =  $BF31                    ;PRODOS DEVICE COUNT
DEVLIST  =  $BF32                    ;LIST OF DEVICES FOR PRODOS
DEVADR   =  $BF10                    ;GIVEN _SLOT THIS IS THE ADDRESS OF DRIVER
BUFFER   =  $EE                       ;ADDRESS POINTER FOR FORMAT DATA
CH       =  $24                      ;STORAGE FOR HORIZONTAL CURSOR VALUE
IN       =  $200                     ;KEYBOARD INPUT BUFFER
WARMDOS  =  $BE00                    ;BASIC WARM-START VECTOR
MLI      =  $BF00                    ;PRODOS MACHINE LANGUAGE INTERFACE
LAST     =  $BF30                    ;LAST DEVICE ACCESSED BY PRODOS
;STROUT   =  $DB3A                    ;APPLESOFT'S STRING PRINTER
WAIT     =  $FCA8                    ;DELAY ROUTINE
CLRLN    =  $FC9C                    ;CLEAR LINE ROUTINE
;RDKEY    =  $FD0C                    ;CHARACTER INPUT ROUTINE  (READ KEYBOARD)
PRBYTE   =  $FDDA                    ;PRINT BYTE ROUTINE (HEX VALUE)
;COUT     =  $FDED                    ;CHARACTER OUTPUT ROUTINE (PRINT TO SCREEN)
STEP0    =  (>__FORMAT_LOAD__ + $1780)                    ;DRIVE STEPPER MOTOR POSITIONS
STEP1    =  (>__FORMAT_LOAD__ + $1781)                    ;  |      |      |       |
STEP2    =  (>__FORMAT_LOAD__ + $1782)                    ;  |      |      |       |
STEP4    =  (>__FORMAT_LOAD__ + $1784)                    ;  |      |      |       |
STEP6    =  (>__FORMAT_LOAD__ + $1786)                    ;  |      |      |       |
DISKOFF  =  (>__FORMAT_LOAD__ + $1788)                    ;DRIVE OFF  SOFTSWITCH
DISKON   =  (>__FORMAT_LOAD__ + $1789)                    ;DRIVE ON   SOFTSWITCH
SELECT   =  (>__FORMAT_LOAD__ + $178A)                    ;STARTING OFFSET FOR TARGET DEVICE
DISKRD   =  (>__FORMAT_LOAD__ + $178C)                    ;DISK READ  SOFTSWITCH
DISKWR   =  (>__FORMAT_LOAD__ + $178D)                    ;DISK WRITE SOFTSWITCH
MODERD   =  (>__FORMAT_LOAD__ + $178E)                    ;MODE READ  SOFTSWITCH
MODEWR   =  (>__FORMAT_LOAD__ + $178F)                    ;MODE WRITE SOFTSWITCH
;**********************************************************
; EQUATES
;**********************************************************
STRING = $EC

;***************************
;*                         *
;* _GETYN - GET A YES OR NO *
;* ANSWER FROM THE USER    *
;*                         *
;***************************
.proc _GETYN
         JSR   RDKEY                    ;GET A KEYPRESS
         ;AND   #$DF                     ;MASK LOWERCASE
         CMP   #$59                     ;IS IT A Y?
         BEQ   LSHOW
         LDA   #$4E                     ;OTHERWISE DEFAULT TO "NO"
LSHOW:    JSR   COUT                     ;PRINT CHAR, Z FLAG CONTAINS STATUS
         CMP   #$59                     ;CONDITION FLAG
         RTS
.endproc

.proc COUT
	pha
	jsr _cputc
	pla
	rts
.endproc

.proc RDKEY
	jsr _cgetc
	rts
.endproc

.proc STROUT
	lda STRING
	ldx STRING+1

	jsr _cputs

	rts
.endproc

.proc _FORMATTER
         JSR   HOME                     ;CLEARS SCREEN
		 
         TSX
         STX   STACK
		 LDA   LAST                     ;STORE CURRENT _SLOT/DRIVE # IN _SLOT
         STA   QSLOT                    ;SAVE PRODOS'S LAST DEVICE ACCESSED
         ;JSR   MLI
         ;.BYTE $42
         ;.WORD NETPARMS					;CALL FOR APPLETALK WHICH ISN'T WANTED
         ;BCC   NOTERROR
         ;CMP   #$01                     ;EVEN THOUGH EVERYONE SAID THAT THIS
         ;BEQ   REENTRY                  ; SHOULD HAPPEN I NEVER COULD GET IT.
         ;CMP   #$04                     ;GOT THIS BUT DON'T TRY TO CHANGE THE
         ;BEQ   REENTRY                  ; PARAMETER COUNT TO 1 OR #$%@&*^()
NOTERROR: 
         ;rts
         ;LDA   NETDEV
         ;JSR   HEXDEC
         ;LDA   #<APPLETALK               ;PROMPT TO CONTINUE OR NOT
         ;LDY   #>APPLETALK              ;BECAUSE APPLETALK IS INSTALLED
         ;JSR   STROUT
         ;JSR   _GETYN
         ;BEQ   REENTRY
         ;JMP   MEXIT
REENTRY:  
         JSR   HOME                     ;CLEARS SCREEN
         LDA   #<TARGSLT                 ;PROMPT FOR _SLOT
		 STA   STRING
		 LDA   #>TARGSLT
		 STA   STRING+1
         JSR   STROUT
LSLOT:    JSR   RDKEY                    ;GET A KEYPRESS
		 ;JSR   COUT
         CMP   #$30                     ;LESS THAN _SLOT #1?
         BCC   LSLOT
         CMP   #$38                     ;GREATER THAN _SLOT #7?
         BCS   LSLOT
         STA   BUFFER                   ;STORE _SLOT NUMBER IN BUFFER
         JSR   COUT                     ;PRINT IT ON THE SCREEN
         LDA   #<TARGDRV                 ;PROMPT FOR DRIVE
		 STA   STRING
         LDA   #>TARGDRV
		 STA   STRING + 1
         JSR   STROUT
LDRIVE:   JSR   RDKEY                    ;GET A KEYPRESS
         CMP   #$31                     ;DRIVE #1?
         BEQ   LCONVERT
         CMP   #$32                     ;DRIVE #2?
         BNE   LDRIVE
LCONVERT: STA   BUFFER+1                 ;STORE DRIVE NUMBER IN BUFFER+1
         JSR   COUT                     ;PRINT IT ON THE SCREEN
         JSR   DOTWO                    ;PRINT TWO CARRIAGE RETURNS
         LDA   BUFFER                   ;FETCH THE _SLOT NUMBER
         AND   #$0F                     ;MASK OFF THE UPPER 4 BITS
         ROL   A                        ;MOVE LOWER 4 BITS TO UPPER 4 BITS
         ROL   A
         ROL   A
         ROL   A
         STA   _SLOT                     ;STORE RESULT IN FORMAT _SLOT
         TAX
         LDA   BUFFER+1                 ;FETCH THE DRIVE NUMBER
         CMP   #$B1                     ;DOES _SLOT NEED CONDITIONING?
         BEQ   JUMP5                    ;NOPE
JUMP1:    LDA   _SLOT                     ;FETCH FORMAT _SLOT
         ORA   #$80                     ;SET MSB TO INDICATE DRIVE #2
         STA   _SLOT
         TAX
JUMP5:    LDY   DEVCNT                   ;LOAD HOW MANY DEVICES
FLOOP:    LDA   DEVLIST,Y                ; SINCE THIS ISN'T A SEQUENTIAL
         STA   LISTSLOT                 ; LIST THEN MUST GO THROUGH EACH ONE
         AND   #$F0                     ; MUST ALSO STORE WHAT IS THERE FOR LATER
         CMP   _SLOT
         BEQ   ITISNUM
         DEY
         BPL   FLOOP
         JMP   NOUNIT                   ;USED TO BE BMI
ITISNUM:  
         TXA                            ;MAKE THE _SLOT THE INDEXED REGISTER
         LSR   A                        ; FOR GETTING DEVICE DRIVE CONTROLLER
         LSR   A
         LSR   A
         TAY
         LDA   DEVADR,Y                 ;GET IT
         STA   ADDR                     ; AND SAVE IT
         LDA   DEVADR+1,Y
         STA   ADDR+1
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
         STA   ADDR+1                   ;IF IT ISN'T EITHER THEN TREAT IT AS
         JMP   YESSMART1                ; SMART AND SAVE WHAT BANK IT IS IN.

YESRAM3:  
         LDA   ADDR+1                   ;IF YOU THINK IT IS A /RAM THEN CHECK
         CMP   #$FF                     ; THE BITS THAT TELL YOU SO.
         BEQ   LOOP7                    ; DOES THE ADDRESS POINTER START WITH FF
         JMP   NOUNIT
LOOP7:    LDA   ADDR                     ; AND END WITH 00
         CMP   #$00
         BEQ   LOOP8
         JMP   NOUNIT
LOOP8:    LDA   LISTSLOT
         AND   #$F3
         CMP   #$B3
         BEQ   LOOP9
         JMP   NOUNIT
LOOP9:    LDA   #<ITISRAM3                ;TELL THE PRESON THAT YOU THINK IT IS A
		 STA STRING
         LDA   #>ITISRAM3               ; /RAM AND IF THEY WANT TO CONTINUE
		 STA STRING + 1
         JSR   STROUT
         JSR   _GETYN
         BNE   JUMP2
         JSR   OLDNAME
         JSR   RAM3FORM
JUMP2:    JMP   AGAIN
YESSMART: 
         TYA
         AND   #$0F
         ROL   A                        ;MOVE LOWER 4 BITS TO UPPER 4 BITS
         ROL   A
         ROL   A
         ROL   A
         STA   _SLOT                     ;STORE RESULT IN FORMAT _SLOT
YESSMART1: 
         LDA   ADDR+1                   ;CHECK SIGNITURE BYTES IN THE CN PAGE
         STA   BUFFER+1                 ; FOR A SMART DEVICE.
         LDA   #$00
         STA   BUFFER
         LDY   #$01
         LDA   (BUFFER),Y
         CMP   #$20
         BNE   NOUNIT
         LDY   #$03
         LDA   (BUFFER),Y
         BNE   NOUNIT
         LDY   #$05
         LDA   (BUFFER),Y
         CMP   #$03
         BNE   NOUNIT
         LDY   #$FF
         LDA   (BUFFER),Y
         CMP   #$00                     ;APPLES DISKII
         BEQ   DISKII
         CMP   #$FF                     ;WRONG DISKII
         BEQ   NOUNIT                   ; MUST BE A SMART DEVICE.
         LDY   #$07                     ;TEST LAST SIGNITURE BYTE FOR THE
         LDA   (BUFFER),Y               ; PROTOCOL CONVERTER.
         CMP   #$00
         BNE   NOUNIT                   ;IT ISN'T SO ITS NO DEVICE I KNOW.
         LDA   #<ITISSMART               ;TELL THEM YOU THINK IT IS A SMARTPORT
		 STA   STRING
         LDA   #>ITISSMART              ; DEVICE. AND ASK IF THEY WANT TO FORMAT.
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN
         BNE   JUMP3
         JSR   OLDNAME                  ;SHOW OLD NAME AND ASK IF PROPER DISK
         JSR   LNAME                    ;GET NEW NAME
         JSR   SMARTFORM                ;JUMP TOO ROUTINE TO FORMAT SMART DRIVE
         LDA   LISTSLOT
         AND   #$F0
         STA   _SLOT
         JSR   CODEWR                   ;JUMP TO ROUTINE TO PRODUCE BIT MAP
         JMP   CATALOG                  ;WRITE DIRECTORY INFORMATION TO THE DISK
JUMP3:    JMP   AGAIN


NOUNIT:   LDA   #<UNITNONE                ;PROMPT TO CONTINUE OR NOT BECAUSE
		 STA   STRING
         LDA   #>UNITNONE               ;THERE IS NO UNIT NUMBER LIKE THAT
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN
         BNE   JUMP4
         JMP   REENTRY
JUMP4:    JMP   MEXIT

DISKII:   
         LDA   #<ITSAII                  ;TELL THEM YOU THINK IT IS A DISKII
		 STA   STRING
         LDA   #>ITSAII
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN
         BNE   JUMP3
         LDA   #$18                     ;SET VOLBLKS TO 280 ($118)
         LDX   #$01                     ; JUST SETTING DEFAULT SETTINGS
         LDY   #$00
         STA   VOLBLKS
         STX   VOLBLKS+1
         STY   VOLBLKS+2
         JSR   OLDNAME                  ;PROMPT FOR PROPER DISK.
         JSR   LNAME                    ;GET NEW NAME
         JMP   GO                  ;FORMAT DISKII

LNAME:    LDA   #<VOLNAME
		 STA   STRING
         LDA   #>VOLNAME
		 STA   STRING + 1
         JSR STROUT
LRDNAME:  LDA   #$0E                     ;RESET CH TO 14
         STA   CH
         LDX   #$00
         BEQ   LINPUT                   ;ALWAYS TAKEN
LBACKUP:  CPX   #0                       ;HANDLE BACKSPACES
         BEQ   LRDNAME
         DEX
         LDA   #$88                     ;<--
         JSR   COUT
LINPUT:   JSR   RDKEY                    ;GET A KEYPRESS
         CMP   #$08                     ;BACKSPACE?
         BEQ   LBACKUP
         CMP   #$7F                     ;DELETE?
         BEQ   LBACKUP
         CMP   #$0D                     ;C/R IS END OF INPUT
         BEQ   LFORMAT
         CMP   #$2E                     ;(PERIODS ARE OK...)
         BEQ   LSTORE
         CMP   #$30                     ;LESS THAN '0'?
         BCC   LINPUT
         CMP   #$3A                     ;LESS THAN ':'?
         BCC   LSTORE                   ;VALID. STORE THE KEYPRESS
         AND   #$DF                     ;FORCE ANY LOWER CASE TO UPPER CASE
         CMP   #$40                     ;LESS THAN 'A'?
         BEQ   LINPUT
         CMP   #$5B                     ;GREATER THAN 'Z'?
         BCS   LINPUT
LSTORE:   JSR   COUT                     ;PRINT KEYPRESS ON THE SCREEN
         AND   #$7F                     ;CLEAR MSB
         STA   VOLNAM,X                 ;STORE CHARACTER IN VOLNAM
         INX
         CPX   #$0E                     ;HAVE 15 CHARACTERS BEEN ENTERED?
         BCC   LINPUT
LFORMAT:  TXA                            ;SEE IF DEFAULT VOLUME_NAME WAS TAKEN
         BNE   LSETLEN
WLOOP:    LDA   BLANK,X                  ;TRANSFER 'BLANK' TO VOLNAM
         AND   #$7F                     ;CLEAR MSB
         STA   VOLNAM,X
         INX
         CPX   #$05                     ;END OF TRANSFER?
         BCC   WLOOP
         LDA   #$13                     ;RESET CH TO 19
         STA   CH
LSETLEN:  JSR   CLRLN                    ;ERASE THE REST OF THE LINE
         CLC
         TXA                            ;ADD $F0 TO VOLUME NAME LENGTH
         ADC   #$F0                     ;CREATE STORAGE_TYPE, NAME_LENGTH BYTE
         STA   VOLLEN
         RTS

GO:
		 JSR   FORMAT                   ;FORMAT THE DISK
         ;JSR   CODEWR                   ;FORM BITMAP

         LDA   #<VERIF                   ;ASK IF YOU WANT TO VERIFY THE DISK
		 STA   STRING
         LDA   #>VERIF
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN                    ;GET A YES OR NO ANSWER TO 'VERIFY?'
         BNE   BIIFORM1
         JMP   VERIFY                   ;ANSWER WAS YES...
BIIFORM1: JMP   CATALOG                  ;WRITE DIRECTORY INFORMATION TO THE DISK

CODEWR:   LDA   #$81                     ;SET OPCODE TO WRITE
         STA   OPCODE
;**********************************
;*                                *
;* WRITE BLOCK0 TO DISK           *
;*                                *
;**********************************
ASKBLK0:  LDA   #<BOOTCODE                ;SET MLIBUF TO BOOTCODE
         LDY   #>BOOTCODE
         STA   MLIBUF
         STY   MLIBUF+1
         LDA   #$00                     ;SET MLIBLK TO 0
         STA   MLIBLK
         STA   MLIBLK+1
         JSR   CALLMLI                  ;WRITE BLOCK #0 TO TARGET DISK
;**************************************
;* FILL BUFFER $6800-$69FF WITH ZEROS *
;* AND PREPARE BITMAP AND LINK BLOCKS *
;* FOR WRITING TO DISK                *
;*                                    *
;**************************************
FILL:     
         LDA   #$05                     ;BLOCK 5 ON DISK
         STA   MLIBLK
         LDA   #$00                     ;SET BUFFER, MLIBUF TO $6800
         LDX   #(>__FORMAT_LOAD__ + $18)
         STA   MLIBUF
         STA   BUFFER
         STX   MLIBUF+1
         STX   BUFFER+1
         TAY                            ;FILL $6800-$69FF WITH ZEROS
         LDX   #$01                     ;FILL 2 PAGES OF 256 BYTES
LZERO:    STA   (BUFFER),Y
         INY
         BNE   LZERO
         INC   BUFFER+1
         DEX
         BPL   LZERO
         LDA   #(>__FORMAT_LOAD__ + $18)                     ;RESET BUFFER TO $6800
         STA   BUFFER+1
         LDA   #$05                     ;LENGTH OF DIRTBL
         STA   COUNT
LLINK:    LDX   COUNT
         LDA   DIRTBL,X                 ;MOVE DIRECTORY LINK VALUES INTO BUFFER
         STA   $6802                    ;STORE NEXT DIRECTORY BLOCK #
         DEX
         LDA   DIRTBL,X                 ;FETCH ANOTHER # FROM DIRTBL
         STA   $6800                    ;STORE PREVIOUS DIRECTORY BLOCK #
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
         JMP   DIED
BITMAPCODE: 
         LDA   #%00000001               ;CLEAR FIRST 7 BLOCKS
         STA   (BUFFER),Y
         LDY   COUNT+1                  ;ORIGINAL LOW BLOCK COUNT VALUE
         BNE   JUMP11                   ;IF IT IS 0 THEN MAKE FF
         DEY                            ;MAKE FF
         DEC   COUNT+2                  ;MAKE 256 BLOCKS LESS ONE
         STY   COUNT+1                  ;MAKE FF NEW LOW BLOCK VALUE
JUMP11:   
         LDX   COUNT+2                  ;HIGH BLOCK VALUE
         BNE   JUMP15                   ;IF IT ISN'T EQUAL TO 0 THEN BRANCH
         LDY   COUNT+1
         JMP   JUMP19

JUMP15:   
         LDA   #(>__FORMAT_LOAD__ + $19)                      ;SET THE ADRESS OF THE UPPER PART OF
         STA   BUFFER+1                 ; BLOCK IN BITMAP BEING CREATED
         LDA   #%11111111
         LDY   COUNT+1                  ;USING THE LOW BYTE COUNT
JUMP20:   DEY
         STA   (BUFFER),Y               ;STORE THEM
         BEQ   JUMP17
         JMP   JUMP20
JUMP17:   
         DEY                            ;FILL IN FIRST PART OF BLOCK
         LDA   #(>__FORMAT_LOAD__ + $18) 
         STA   BUFFER+1
JUMP19:   LDA   #%11111111
         DEY
         STA   (BUFFER),Y
         CPY   #$01                     ;EXCEPT THE FIRST BYTE.
         BEQ   JUMP18
         JMP   JUMP19
JUMP18:   RTS



;*************************************
;*                                   *
;* VERIFY - VERIFY EACH BLOCK ON THE *
;* DISK, AND FLAG BAD ONES IN BITMAP *
;*                                   *
;*************************************
VERIFY:   
         LDA   #$80                     ;SET OPCODE TO $80 (READ)
         STA   OPCODE
         LDA   #$60                     ;CHANGE ERROR TO AN RTS INSTRUCTION
         STA   ERROR
         LDA   #$00                     ;RESET MLIBUF TO $1000
         LDX   #$10
         STA   MLIBUF
         STX   MLIBUF+1
         STA   COUNT                    ;SET COUNT AND POINTER TO 0
         STA   POINTER
         STA   LBAD                     ;SET BAD BLOCK COUNTER TO 0
         STA   MLIBLK                   ;RESET MLIBLK TO 0
         STA   MLIBLK+1
LREAD:    JSR   CALLMLI                  ;READ A BLOCK
         BCS   LERROR                   ;UPDATE BITMAP IF ERROR OCCURS
LINCBLK:  CLC                            ;ADD 1 TO MLIBLK
         INC   MLIBLK
         BNE   LCHECK
         INC   MLIBLK+1
LCHECK:   INC   COUNT                    ;ADD 1 TO BITMAP COUNTER
         LDA   COUNT                    ;IF COUNT > 7 THEN ADD 1 TO POINTER
         CMP   #$08
         BCC   MDONE
         LDA   #$00                     ;RESET COUNT TO 0
         STA   COUNT
         INC   POINTER                  ;ADD 1 TO POINTER OFFSET VALUE
MDONE:    LDX   MLIBLK                   ;SEE IF WE'VE READ 280 BLOCKS YET
         LDA   MLIBLK+1
         BEQ   LREAD
         CPX   #$18                     ;GREATER THAN $118 (280) BLOCKS READ?
         BEQ   LRESULT                  ;FINISHED. DISPLAY RESULTS OF VERIFY
         BNE   LREAD                    ;GO GET ANOTHER BLOCK
LERROR:   LDX   COUNT                    ;USE COUNT AS OFFSET INTO MAPTBL
         LDA   BITTBL,X                 ;FETCH VALUE FOR BAD BLOCK NUMBER
         LDY   POINTER                  ;USE POINTER AS OFFSET INTO BUFFER
         AND   (BUFFER),Y               ;MASK VALUE AGAINST BITMAP VALUE
         STA   (BUFFER),Y               ;STORE NEW BITMAP VALUE IN BUFFER
         CLC
         DEC   VOLBLKS                  ;DECREMENT # OF BLOCKS AVAILABLE
         BNE   LINCBAD
         DEC   VOLBLKS+1
LINCBAD:  INC   LBAD                     ;ADD 1 TO # OF BAD BLOCKS FOUND
         JMP   LINCBLK                  ;GET THE NEXT BLOCK ON THE DISK
LRESULT:  LDA   #$EA                     ;CHANGE ERROR BACK TO A NOP INSTRUCTION
         STA   ERROR
         LDA   LBAD                     ;FIND OUT IF THERE WERE ANY BAD BLOCKS
         BEQ   LGOOD
         JSR   HEXDEC                   ;CONVERT HEX NUMBER INTO DECIMAL
         LDX   #$00
BLOOP:    LDA   IN,X                     ;PRINT THE DECIMAL =IVALENT OF LBAD
         CMP   #$31                     ;DON'T PRINT ZEROS...
         BCC   MNEXT
         JSR   COUT
MNEXT:    INX
         CPX   #$03                     ;END OF NUMBER?
         BCC   BLOOP
         LDA   #<BAD
		 STA   STRING
         LDA   #>BAD
		 STA   STRING + 1
         JSR   STROUT
         JMP   CATALOG                  ;WRITE BITMAP AND LINKS TO THE DISK
LGOOD:    LDA   #<GOOD
		 STA   STRING
         LDA   #>GOOD
		 STA   STRING + 1
         JSR   STROUT
         JMP   CATALOG
LBAD:     .BYTE $00                        ;NUMBER OF BAD BLOCKS FOUND
;*************************************
;*                                   *
;* CATALOG - BUILD A DIRECTORY TRACK *
;*                                   *
;*************************************
CATALOG:  
         LDA   #$81                     ;CHANGE OPCODE TO $81 (WRITE)
         STA   OPCODE
         LDA   #$00                     ;RESET MLIBUF TO $6800
         LDX   #(>__FORMAT_LOAD__ + $18) 
         STA   MLIBUF
         STX   MLIBUF+1
         LDX   #$06                     ;WRITE BUFFER (BITMAP) TO BLOCK #6 ON THE DISK
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
         LDY   #$2A                     ;MOVE BLOCK2 INFORMATION TO $6800
CLOOP:    LDA   BLOCK2,Y
         STA   (BUFFER),Y
         DEY
         BPL   CLOOP
         LDA   #$02                     ;WRITE BLOCK #2 TO THE DISK
         STA   MLIBLK
         JSR   CALL2MLI
AGAIN:    
         LDA   #<NUTHER                  ;DISPLAY 'FORMAT ANOTHER' STRING
		 STA   STRING
         LDA   #>NUTHER
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN                    ;GET A YES OR NO ANSWER
         BNE   MEXIT                    ;ANSWER WAS NO...
         JMP   REENTRY                  ;FORMAT ANOTHER DISK
MEXIT:    JSR   DOTWO                    ;TWO FINAL CARRIAGE RETURNS...
         LDA   QSLOT
         STA   LAST
         LDX   STACK                    ;JUST BECAUSE I AM HUMAN TO AND MIGHT
         TXS                            ;HAVE MESSED UP ALSO.
         JMP   WARMDOS                  ;EXIT TO BASIC
CALL2MLI: 
         JSR   CALLMLI
         RTS

;*************************************
;*                                   *
;* CALLMLI - CALL THE MLI READ/WRITE *
;* ROUTINES TO TRANSFER BLOCKS TO OR *
;* FROM MEMORY                       *
;*                                   *
;*************************************
CALLMLI:  NOP

         JSR   MLI                      ;CALL THE PRODOS MACHINE LANGAUGE INTERFACE
OPCODE:  .BYTE $81                  ;DEFAULT MLI OPCODE = $81 (WRITE)
         .WORD PARMS
         BCS   ERROR
         RTS
ERROR:                               ;(THIS WILL BE CHANGED TO RTS BY VERIFY)
         PLA
         PLA
         PLA
         PLA
         JMP   DIED
;**************************************
;*                                    *
;* DOTWO - PRINT TWO CARRIAGE RETURNS *
;*                                    *
;**************************************

DOTWO:    
         LDA   #$0D                     ;(WE DON'T NEED AN EXPLANATION, DO WE?)
         JSR   COUT
		 LDA   #$0A
         JSR   COUT
         LDA   #$0D                     ;(WE DON'T NEED AN EXPLANATION, DO WE?)
         JSR   COUT
		 LDA   #$0A
         JSR   COUT
         RTS
;***********************************
;*                                 *
;* HEXDEC - CONVERT HEX TO DECIMAL *
;*                                 *
;***********************************

HEXDEC:   
         STA   IN+20                    ;STORE NUMBER IN KEYBOARD INPUT BUFFER
         LDA   #$00
         STA   IN+21
         LDY   #$02                     ;RESULT WILL BE THREE DIGITS LONG
DLOOP:    LDX   #$11                     ;16 BITS TO PROCESS
         LDA   #$00
         CLC
LDIVIDE:  ROL   A
         CMP   #$0A                     ;VALUE > OR = TO 10?
         BCC   LPLUS
         SBC   #$0A                     ;SUBTRACT 10 FROM THE VALUE
LPLUS:    ROL   IN+20                    ;SHIFT VALUES IN IN+20, IN+21 ONE BIT LEFT
         ROL   IN+21
         DEX
         BNE   LDIVIDE
         ORA   #$B0                     ;CONVERT VALUE TO HIGH ASCII CHARACTER
         STA   IN,Y                     ;STORE IT IN THE INPUT BUFFER
         STA   NETNUM,Y
         DEY
         BPL DLOOP
         RTS
;***********************************
;*                                 *
;* FORMAT - FORMAT THE TARGET DISK *
;*                                 *
;***********************************

FORMAT:   
         PHP
         SEI
         LDA   _SLOT                     ;FETCH TARGET DRIVE SLOTNUM VALUE
         PHA                            ;STORE IT ON THE STACK
         AND   #$70                     ;MASK OFF BIT 7 AND THE LOWER 4 BITS
         STA   SLOTF                    ;STORE RESULT IN FORMAT _SLOT STORAGE
         TAX                            ;ASSUME VALUE OF $60 (DRIVE #1)
         PLA                            ;RETRIEVE VALUE FROM THE STACK
         BPL   LDRIVE1                  ;IF < $80 THE DISK IS IN DRIVE #1
         INX                            ;SET X OFFSET TO $61 (DRIVE #2)
LDRIVE1:  LDA   SELECT,X                 ;SET SOFTSWITCH FOR PROPER DRIVE
         LDX   SLOTF                    ;SET X OFFSET TO FORMAT _SLOT/DRIVE
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
         PLP
         RTS                            ;FORMAT IS FINISHED. RETURN TO CALLING ROUTINE
DIEDII:   PHA                            ;SAVE MLI ERROR CODE ON THE STACK
         JSR   DONE
         PLA                            ;RETRIEVE ERROR CODE FROM THE STACK
         JMP   DIED                     ;PROMPT FOR ANOTHER FORMAT...

;**************************************
;*                                    *
;* DIED - SOMETHING AWFUL HAPPENED TO *
;* THE DISK OR DRIVE. DIE A MISERABLE *
;* DEATH...                           *
;*                                    *
;**************************************
DIED:     
         CMP   #$4D                     ;SAVE MLI ERROR CODE ON THE STACK
         BEQ   RANGEERROR
         CMP   #$27
         BEQ   DRIVEOPEN
         CMP   #$2F
         BEQ   DISKERROR
         CMP   #$2B
         BEQ   PROTECTED
         JMP   NODIED
RANGEERROR: LDA   #<TOOLARGE
         LDY   #>TOOLARGE
         JMP   DIEDOUT
DISKERROR: LDA   #<NODISK
         LDY   #>NODISK
         JMP   DIEDOUT
DRIVEOPEN: LDA   #<DEAD
         LDY   #>DEAD
         JMP   DIEDOUT
PROTECTED: LDA   #<PROTECT
         LDY   #>PROTECT
         JMP   DIEDOUT
NODIED:   PHA                            ;SAVE MLI ERROR CODE ON THE STACK
         LDA   #<UNRECOG
		 STA   STRING
         LDA   #>UNRECOG
		 STA   STRING + 1
         JSR   STROUT
         PLA                            ;RETRIEVE ERROR CODE FROM THE STACK
         JSR   PRBYTE                   ;PRINT THE MLI ERROR CODE
         JMP   AGAIN
DIEDOUT:  JSR   STROUT
         JMP   AGAIN                    ;PROMPT FOR ANOTHER FORMAT...
;************************************
;*                                  *
;* TRANS - TRANSFER TRACK IN MEMORY *
;* TO TARGET DEVICE                 *
;*                                  *
;************************************

TRANS:    
         LDA   #$00                     ;SET BUFFER TO $6700
         LDX   #$67
         STA   BUFFER
         STX   BUFFER+1
         LDY   #$32                     ;SET Y OFFSET TO 1ST SYNC BYTE (MAX=50)
         LDX   SLOTF                    ;SET X OFFSET TO FORMAT _SLOT/DRIVE
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
LSYNC3:   LDA   (BUFFER),Y               ;FETCH BYTE TO WRITE TO DISK
         CMP   #$80                     ;IS IT A SYNC BYTE? ($7F)
         BCC   LSYNC1                   ;YEP. TURN IT INTO AN $FF
         NOP
MSTORE:   STA   DISKWR,X                 ;WRITE BYTE TO THE DISK
         CMP   DISKRD,X                 ;SET READ SOFTSWITCH
         INY                            ;INCREMENT Y OFFSET
         BNE   LSYNC2
         INC   BUFFER+1                 ;INCREMENT BUFFER BY 255
         BPL   LSYNC3                   ;IF < $8000 GET MORE FORMAT DATA
         LDA   MODERD,X                 ;RESTORE MODE SOFTSWITCH TO READ
         LDA   DISKRD,X                 ;RESTORE READ SOFTSWITCH TO READ
         CLC
         RTS
LWRPROT:  CLC                            ;DISK IS WRITE PROTECTED! (NERD!)
         JSR   DONE                     ;TURN THE DRIVE OFF
         LDA   #$2B
         PLA
         PLA
         PLA
         PLA
         JMP   DIED                     ;PROMPT FOR ANOTHER FORMAT...
;************************************
;*                                  *
;* BUILD - BUILD GAP1 AND 16 SECTOR *
;* IMAGES BETWEEN $6700 AND $8000   *
;*                                  *
;************************************
BUILD:    
         LDA   #$10                     ;SET BUFFER TO $6710
         LDX   #$67
         STA   BUFFER
         STX   BUFFER+1
         LDY   #$00                     ;(Y OFFSET ALWAYS ZERO)
         LDX   #$F0                     ;BUILD GAP1 USING $7F (SYNC BYTE)
         LDA   #$7F
         STA   LBYTE
         JSR   LFILL                    ;STORE SYNC BYTES FROM $6710 TO $6800
         LDA   #$10                     ;SET COUNT FOR 16 LOOPS
         STA   COUNT
LIMAGE:   LDX   #$00                     ;BUILD A SECTOR IMAGE IN THE BUFFER AREA
ELOOP:    LDA   LADDR,X                  ;STORE ADDRESS HEADER, INFO & SYNC BYTES
         BEQ   LINFO
         STA   (BUFFER),Y
         JSR   LINC                     ;ADD 1 TO BUFFER OFFSET ADDRESS
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
         STA   (BUFFER),Y
         JSR   LINC
         INX
         BNE   YLOOP
LDECCNT:  CLC
         DEC   COUNT
         BNE   LIMAGE
         RTS                            ;RETURN TO WRITE TRACK TO DISK (WRITE)
LFILL:    LDA   LBYTE
         STA   (BUFFER),Y               ;MOVE A REGISTER TO BUFFER AREA
         JSR   LINC                     ;ADD 1 TO BUFFER OFFSET ADDRESS
         DEX
         BNE   LFILL
         RTS
LINC:     CLC
         INC   BUFFER                   ;ADD 1 TO BUFFER ADDRESS VECTOR
         BNE   LDONE
         INC   BUFFER+1
LDONE:    RTS
;***********************************
;*                                 *
;* CALC - CALCULATE TRACK, SECTOR, *
;* AND CHECKSUM VALUES OF THE NEXT *
;* TRACK USING 4&4 ENCODING        *
;*                                 *
;***********************************
CALC:     
         LDA   #$03                     ;SET BUFFER TO $6803
         LDX   #(>__FORMAT_LOAD__ + $18) 
         STA   BUFFER
         STX   BUFFER+1
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
         CLC                            ;ADD 385 ($181) TO BUFFER ADDRESS
         LDA   BUFFER
         ADC   #$81
         STA   BUFFER
         LDA   BUFFER+1
         ADC   #$01
         STA   BUFFER+1
         INC   SECTOR                   ;ADD 1 TO SECTOR VALUE
         LDA   SECTOR                   ;IF SECTOR > 16 THEN QUIT
         CMP   #$10
         BCC   ZLOOP
         RTS                            ;RETURN TO WRITE TRACK TO DISK (WRITE)
LENCODE:  PHA                            ;PUT VALUE ON THE STACK
         LSR   A                        ;SHIFT EVERYTHING RIGHT ONE BIT
         ORA   #$AA                     ;OR IT WITH $AA
         STA   (BUFFER),Y               ;STORE 4&4 RESULT IN BUFFER AREA
         INY
         PLA                            ;RETRIEVE VALUE FROM THE STACK
         ORA   #$AA                     ;OR IT WITH $AA
         STA   (BUFFER),Y               ;STORE 4&4 RESULT IN BUFFER AREA
         INY
         RTS
;*************************************
;*                                   *
;* SEEK - MOVE HEAD TO DESIRED TRACK *
;*                                   *
;*************************************
SEEK:     
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

PHASE:    
         ORA   SLOTF                    ;OR _SLOT VALUE TO PHASE
         TAX
         LDA   STEP1,X                  ;PHASE ON...
         LDA   #$56                     ;20 MS. DELAY
         JSR   WAIT
         LDA   STEP0,X                  ;PHASE OFF...
         RTS
;**********************************
;*                                *
;* FORMAT A RAM3 DEVICE.          *
;*                                *
;**********************************
RAM3FORM: 
         PHP
         SEI
         LDA   #3                       ;FORMAT REQUEST NUMBER
         STA   $42
         LDA   _SLOT                     ;_SLOT OF /RAM
         STA   $43
         LDA   #$00                     ;BUFFER SPACE IF NEEDED LOW BYTE
         STA   $44
         LDA   #$67                     ; AND HIGH BYTE
         STA   $45

         LDA   $C08B                    ;READ AND WRITE RAM, USING BANK 1
         LDA   $C08B

         JSR   RAM3DRI
         BIT   $C082                    ;READ ROM, USE BANK 2(PUT BACK ON LINE)
         BCS   RAM3ERR
         PLP
         RTS

RAM3DRI:  JMP   (ADDR)
RAM3ERR:  
         TAX
         PLP
         PLA
         PLA
         TXA
         JMP   DIED
;**********************************
;*                                *
;* FORMAT A SMARTPORT DEVICE      *
;*                                *
;**********************************
SMARTFORM: 
         PHP
         SEI
         LDA   #0                       ;REQUEST PROTOCOL CONVERTER FOR A STATUS
         STA   $42
         LDA   LISTSLOT                 ;GIVE IT THE PRODOS _SLOT NUMBER
         AND   #$F0
         STA   $43
         LDA   #$00                     ;GIVE IT A BUFFER MAY NOT BE NEEDED BY
         STA   $44                      ; GIVE IT TO IT ANYWAYS
         LDA   #(>__FORMAT_LOAD__ + $18) 
         STA   $45
         LDA   #$03                     ;THE BLOCKS OF DEVICE
         STA   $46
         JSR   SMARTDRI
         TXA                            ;LOW IN X REGISTER
         STA   VOLBLKS                  ; SAVE IT
         TYA                            ; HIGH IN Y REGISTER
         STA   VOLBLKS+1                ; SAVE IT
         LDA   #$00
         STA   VOLBLKS+2
         LDA   #3                       ;GIVE PROTOCOL CONVERTER A FORMAT REQUEST
         STA   $42                      ;GIVE IT _SLOT NUMBER
         LDA   LISTSLOT
         AND   #$F0
         STA   $43
         LDA   #$00                     ;GIVE A BUFFER WHICH PROBABLY WON'T BE
         STA   $44                      ; USED.
         LDA   #(>__FORMAT_LOAD__ + $18) 
         STA   $45

         JSR   SMARTDRI
         BCS   SMARTERR
         PLP
         RTS

SMARTDRI: JMP   (ADDR)
SMARTERR: 
         TAX
         PLP
         PLA
         PLA
         TXA
         JMP   DIED

;**********************************
;*                                *
;* IS THERE AN OLD NAME?          *
;*                                *
;**********************************
OLDNAME:  
         LDA   LISTSLOT
         AND   #$F0
         STA   INFO+1
         JSR   MLI
         .BYTE $C5
         .WORD INFO
         LDA   VOLLEN
         AND   #$0F
         BNE   OLDNAME1
         LDA   VOLLEN+1
         CMP   #$28
         BNE   OLDERROR
         PLA
         PLA
         JMP   NOUNIT
OLDNAME1: STA   VOLLEN
         LDA   #<THEOLD1
		 STA   STRING
         LDA   #>THEOLD1
		 STA   STRING + 1
         JSR   STROUT
         LDA   #<VOLLEN                  ;GET NAME LENGTH
		 STA   STRING
         LDA   #>VOLLEN
		 STA   STRING + 1
         JSR   STROUT                   ;PRINT OLD NAME
         LDA   #<THEOLD2
		 STA   STRING
         LDA   #>THEOLD2
		 STA   STRING + 1
         JSR   STROUT
         JSR   _GETYN
         BEQ   OLDERROR
         PLA
         PLA
         JMP   AGAIN
OLDERROR: 
         RTS
.endproc
;*************************
;*                       *
;* VARIABLE STORAGE AREA *
;*                       *
;*************************

INFO:     
         .BYTE $02
         .BYTE $00
         .WORD VOLLEN
PARMS:    .BYTE $03                  ;PARAMETER COUNT = 3
_SLOT:     .BYTE $60                  ;DEFAULT TO S6,D1
MLIBUF:   .WORD BOOTCODE             ;DEFAULT BUFFER ADDRESS
MLIBLK:   .WORD $0000                ;DEFAULT BLOCK NUMBER OF 0
QSLOT:    .BYTE $00                        ;QUIT _SLOT NUMBER
LISTSLOT: .BYTE $00                        ;SAVING THE _SLOT TOTAL FROM THE LIST
ADDR:     .WORD $0000
NETPARMS: .BYTE $00
         .BYTE  $2F                     ;COMMAND FOR FILISTSESSIONS
         .WORD  $0000                   ;APPLETALK RESULT CODE RETURNED HERE
         .WORD  $100                    ;LENGTH OF STRING
         .WORD  (__FORMAT_LOAD__ + $1700)                   ;BUFFER LOW WORD
         .WORD  $0000                   ;BUFFER HIGH WORD
NETDEV:   .BYTE $00                  ;NUMBER OF ENTRIES RETURNED HERE
LBYTE:    .BYTE $00                        ;STORAGE FOR BYTE VALUE USED IN FILL
LADDR:    .BYTE $D5, $AA, $96              ;ADDRESS HEADER
         .BYTE $AA, $AA, $AA, $AA, $AA, $AA, $AA, $AA                ;VOLUME #, TRACK, SECTOR, CHECKSUM
         .BYTE $DE, $AA, $EB              ;ADDRESS TRAILER
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F, $7F                 ;GAP2 SYNC BYTES
         .BYTE $D5, $AA, $AD              ;BUFFER HEADER
         .BYTE $00                    ;END OF ADDRESS INFORMATION
LDATA:   .BYTE $DE, $AA, $EB              ;DATA TRAILER
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F, $7F, $7F                ;GAP3 SYNC BYTES
         .BYTE $7F, $7F, $7F, $7F, $7F, $7F, $7F, $7F                ;GAP3 SYNC BYTES
         .BYTE $00                    ;END OF DATA INFORMATION

LINOUT:   .BYTE $00                        ;INWARD/OUTWARD PHASE FOR STEPPER MOTOR
LTABLE:   .BYTE $02, $04, $06, $00              ;PHASES FOR MOVING HEAD INWARD
         .BYTE $06, $04, $02, $00              ;   |    |    |      |  OUTWARD

TARGSLT:  .ASCIIZ "FORMAT DISK IN SLOT "
TARGDRV: .ASCIIZ " DRIVE "
VOLNAME: .BYTE $0D, $0A
		 .ASCIIZ "VOLUME NAME: /"
BLANK:   .ASCIIZ "BLANK"
         .ASCIIZ "__________"
VERIF:    .BYTE $0D, $0A, $0D, $0A
		 .BYTE "CERTIFY DISK AND MARK ANY BAD BLOCKS IN "
         .ASCIIZ "THE VOLUME BITMAP AS UNUSABLE? (Y/N): "
THEOLD1:  .BYTE $0D, $0A
	      .BYTE "DO YOU WANT TO WRITE OVER"
		  .BYTE $0D, $0A, $20, $2F, $00
THEOLD2:  .BYTE $20, $2F
		  .ASCIIZ " (Y/N)"
UNRECOG:  .BYTE $0D, $0A
			.ASCIIZ "UNRECOGNISABLE ERROR = "
DEAD:     .BYTE $0D, $0A
			.ASCIIZ "-- CHECK DISK OR DRIVE DOOR --"
PROTECT:  .BYTE $0D, $0A
			.ASCIIZ "DISK IS WRITE PROTECTED, YOU BOZO!"
BAD:      .BYTE $0D, $0A, $0D, $0A
			.ASCIIZ " BAD BLOCK(S) MARKED"
GOOD:     .BYTE $0D, $0A, $0D, $0A
			.ASCIIZ "DISK IS ERROR-FREE"
NODISK:   .BYTE $0D, $0A
			.ASCIIZ "NO DISK IN THE DRIVE"
NUTHER:   .BYTE $0D, $0A, $0D, $0A
			.ASCIIZ "FORMAT ANOTHER DISK? (Y/N): "
TOOLARGE: .BYTE $0D, $0A
			.ASCIIZ "UNIT SIZE IS TO LARGE FOR THIS PROGRAM"
UNITNONE: .BYTE $0D, $0A
			.BYTE "NO UNIT IN THAT _SLOT AND DRIVE", $0D, $0A
         .ASCIIZ "FORMAT ANOTHER DISK? (Y/N): "
ITISRAM3: .BYTE $0D, $0A
			.BYTE "THIS IS A RAM3 DISK", $0D, $0A
         .ASCIIZ "CONTINUE WITH FORMAT? (Y/N): "
ITSAII:   .BYTE $0D, $0A
			.BYTE "THIS IS A DISK II", $0D, $0A
         .ASCIIZ "CONTINUE WITH FORMAT? (Y/N): "
ITISSMART: .BYTE $0D, $0A
			.BYTE "THIS IS A SMARTPORT DEVICE", $0D, $0A
         .BYTE "CONTINUE WITH FORMAT? (Y/N): "
APPLETALK: 
         .BYTE $0D, $0A
			.ASCIIZ "NUMBER OF APPLETALK DEVICES IS = "
NETNUM:   .BYTE $00, $00, $00, $0D, $0A
			.BYTE "APPLETALK IS INSTALLED THIS PROGRAM MAY", $0D, $0A
         .BYTE "NOT WORK PROPERLY DO YOU WANT TO", $0D, $0A
         .ASCIIZ "CONTINUE (Y/N)"
BLOCK2:   .BYTE $00, $00, $03, $00
VOLLEN:   .BYTE $00                        ;$F0 + LENGTH OF VOLUME NAME
VOLNAM:   .BYTE $00, $00, $00, $00, $00, $00, $00, $00 
		  .BYTE $00, $00, $00, $00, $00, $00, $00		;VOLUME NAME, RESERVED, CREATION, VERSION
RESERVED: .BYTE $00, $00, $00, $00, $00, $00
UPLOWCASE: .BYTE $00, $00
DATIME:   .BYTE $00, $00, $00, $00
VERSION:  .BYTE $01
         .BYTE $00, $C3, $27, $0D, $0A
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
SLOTF:    .BYTE $00, $00                        ;_SLOT/DRIVE OF DEVICE TO FORMAT
TRKCUR:   .BYTE $00, $00                        ;CURRENT TRACK POSITION
TRKDES:   .BYTE $00, $00                        ;DESTINATION TRACK POSITION
TRKBEG:   .BYTE $00                   ;STARTING TRACK NUMBER
TRKEND:   .BYTE $35                   ;ENDING TRACK NUMBER
BOOTCODE:
		 .BYTE $01, $38, $B0, $03, $4C, $32, $A1, $86, $43, $C9, $03, $08, $8A, $29, $70, $4A, $4A, $4A, $4A, $09, $C0, $85, $49, $A0
         .BYTE $FF, $84, $48, $28, $C8, $B1, $48, $D0, $3A, $B0, $0E, $A9, $03, $0D, $00, $08, $E6, $3D, $A5, $49, $48, $A9, $5B, $48
         .BYTE $60, $85, $40, $85, $48, $A0, $63, $B1, $48, $99, $94, $09, $C8, $C0, $EB, $D0, $F6, $A2, $06, $BC, $1D, $09, $BD, $24
         .BYTE $09, $99, $F2, $09, $BD, $2B, $09, $9D, $7F, $0A, $CA, $10, $EE, $A9, $09, $85, $49, $A9, $86, $A0, $00, $C9, $F9, $B0
         .BYTE $2F, $85, $48, $84, $60, $84, $4A, $84, $4C, $84, $4E, $84, $47, $C8, $84, $42, $C8, $84, $46, $A9, $0C, $85, $61, $85
         .BYTE $4B, $20, $12, $09, $B0, $68, $E6, $61, $E6, $61, $E6, $46, $A5, $46, $C9, $06, $90, $EF, $AD, $00, $0C, $0D, $01, $0C
         .BYTE $D0, $6D, $A9, $04, $D0, $02, $A5, $4A, $18, $6D, $23, $0C, $A8, $90, $0D, $E6, $4B, $A5, $4B, $4A, $B0, $06, $C9, $0A
         .BYTE $F0, $55, $A0, $04, $84, $4A, $AD, $02, $09, $29, $0F, $A8, $B1, $4A, $D9, $02, $09, $D0, $DB, $88, $10, $F6, $29, $F0
         .BYTE $C9, $20, $D0, $3B, $A0, $10, $B1, $4A, $C9, $FF, $D0, $33, $C8, $B1, $4A, $85, $46, $C8, $B1, $4A, $85, $47, $A9, $00
         .BYTE $85, $4A, $A0, $1E, $84, $4B, $84, $61, $C8, $84, $4D, $20, $12, $09, $B0, $17, $E6, $61, $E6, $61, $A4, $4E, $E6, $4E
         .BYTE $B1, $4A, $85, $46, $B1, $4C, $85, $47, $11, $4A, $D0, $E7, $4C, $00, $20, $4C, $3F, $09, $26, $50, $52, $4F, $44, $4F
         .BYTE $53, $20, $20, $20, $20, $20, $20, $20, $20, $20, $A5, $60, $85, $44, $A5, $61, $85, $45, $6C, $48, $00, $08, $1E, $24
         .BYTE $3F, $45, $47, $76, $F4, $D7, $D1, $B6, $4B, $B4, $AC, $A6, $2B, $18, $60, $4C, $BC, $09, $A9, $9F, $48, $A9, $FF, $48
         .BYTE $A9, $01, $A2, $00, $4C, $79, $F4, $20, $58, $FC, $A0, $1C, $B9, $50, $09, $99, $AE, $05, $88, $10, $F7, $4C, $4D, $09
         .BYTE $AA, $AA, $AA, $A0, $D5, $CE, $C1, $C2, $CC, $C5, $A0, $D4, $CF, $A0, $CC, $CF, $C1, $C4, $A0, $D0, $D2, $CF, $C4, $CF
         .BYTE $D3, $A0, $AA, $AA, $AA, $A5, $53, $29, $03, $2A, $05, $2B, $AA, $BD, $80, $C0, $A9, $2C, $A2, $11, $CA, $D0, $FD, $E9
         .BYTE $01, $D0, $F7, $A6, $2B, $60, $A5, $46, $29, $07, $C9, $04, $29, $03, $08, $0A, $28, $2A, $85, $3D, $A5, $47, $4A, $A5
         .BYTE $46, $6A, $4A, $4A, $85, $41, $0A, $85, $51, $A5, $45, $85, $27, $A6, $2B, $BD, $89, $C0, $20, $BC, $09, $E6, $27, $E6
         .BYTE $3D, $E6, $3D, $B0, $03, $20, $BC, $09, $BC, $88, $C0, $60, $A5, $40, $0A, $85, $53, $A9, $00, $85, $54, $A5, $53, $85
         .BYTE $50, $38, $E5, $51, $F0, $14, $B0, $04, $E6, $53, $90, $02, $C6, $53, $38, $20, $6D, $09, $A5, $50, $18, $20, $6F, $09
         .BYTE $D0, $E3, $A0, $7F, $84, $52, $08, $28, $38, $C6, $52, $F0, $CE, $18, $08, $88, $F0, $F5, $BD, $8C, $C0, $10, $FB, $00
         .BYTE $00, $00, $00, $00, $00, $00, $00, $00
