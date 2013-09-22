*********************************************************************
*                                 *            Movie:               *
*       ProDOS Hyper-FORMAT       *     ProDOS Hyper-FORMAT II      *
*                                 *           The Return            *
*     created by Jerry Hewett     *   Modified by Gary Desrochers   *
*         copyright  1985         *                                 *
*     Living Legends Software     *        Tryll Software           *
*                                 *                                 *
* A Public Domain disk formatting * A Puplic Domain Disk Formatting *
* routine for the ProDOS Disk Op- * You know the same as -------|   *
* erating System.  These routines *<----------------------------|   *
* can be included within your own * Except it now includes code for *
* software as long as you give us * the 3.5 Disk and the Ram3 Disk. *
* credit for developing them.     *  Please don't use this for any  *
*                                 * SmartPort volumes bigger than   *
*       Updated on: 23Aug85       * 2.1 Meg. Look under Hyper-FORMAT*
*                                 * in you local BB Library for the *
*********************************** original code for Hyper-FORMAT  *
;                                 *                                 *
         Keep   HypeForm          *      Updated on: 22Dec89        *
         65816 off                *                                 *
         65C02 off                ***********************************
Home     gequ  $FC58                    ;Monitor clear screen and home cursor
DevCnt   gequ  $BF31                    ;Prodos device count
DevList  gequ  $BF32                    ;List of devices for ProDOS
DevAdr   gequ  $BF10                    ;Given slot this is the address of driver
Buffer   Gequ  $0                       ;Address pointer for FORMAT data
CH       Gequ  $24                      ;Storage for Horizontal Cursor value
IN       Gequ  $200                     ;Keyboard input buffer
WARMDOS  Gequ  $BE00                    ;BASIC Warm-start vector
MLI      Gequ  $BF00                    ;ProDOS Machine Language Interface
LAST     Gequ  $BF30                    ;Last device accessed by ProDOS
STROUT   Gequ  $DB3A                    ;Applesoft's string printer
WAIT     Gequ  $FCA8                    ;Delay routine
CLRLN    Gequ  $FC9C                    ;Clear Line routine
RDKEY    Gequ  $FD0C                    ;Character input routine  (read keyboard)
PRBYTE   Gequ  $FDDA                    ;Print Byte routine (HEX value)
COUT     Gequ  $FDED                    ;Character output routine (print to screen)
Step0    Gequ  $C080                    ;Drive stepper motor positions
Step1    Gequ  $C081                    ;  |      |      |       |
Step2    Gequ  $C082                    ;  |      |      |       |
Step4    Gequ  $C084                    ;  |      |      |       |
Step6    Gequ  $C086                    ;  |      |      |       |
DiskOFF  Gequ  $C088                    ;Drive OFF  softswitch
DiskON   Gequ  $C089                    ;Drive ON   softswitch
Select   Gequ  $C08A                    ;Starting offset for target device
DiskRD   Gequ  $C08C                    ;Disk READ  softswitch
DiskWR   Gequ  $C08D                    ;Disk WRITE softswitch
ModeRD   Gequ  $C08E                    ;Mode READ  softswitch
ModeWR   Gequ  $C08F                    ;Mode WRITE softswitch
;**********************************************************
; Equates
;**********************************************************
         Org   $5000
HypForm  Start
         tsx
         stx   Stack
         LDA   LAST                     ;Store current slot/drive # in Slot
         STA   QSlot                    ;Save Prodos's last device accessed
         JSR   Home                     ;Clears screen
         jsr   MLI
         dc    i1'$42'
         dc    i2'NetParms'             ;Call for Appletalk which isn't wanted
         bcc   NotError
         cmp   #$01                     ;Even though everyone said that this
         beq   Reentry                  ; should happen I never could get it.
         cmp   #$04                     ;Got this but don't try to change the
         beq   Reentry                  ; parameter count to 1 or #$%@&*^()
NotError anop
         lda   NetDev
         jsr   HexDec
         LDA   #Appletalk               ;Prompt to continue or not
         LDY   #>Appletalk              ;because Appletalk is installed
         JSR   STROUT
         jsr   GetYN
         beq   Reentry
         jmp   MExit
Reentry  anop
         JSR   Home                     ;Clears screen
         LDA   #TargSlt                 ;Prompt for slot
         LDY   #>TargSlt
         JSR   STROUT
LSlot    JSR   RDKEY                    ;Get a keypress
         CMP   #$B0                     ;Less than slot #1?
         BCC   LSlot
         CMP   #$B8                     ;Greater than slot #7?
         BCS   LSlot
         STA   Buffer                   ;Store SLOT number in Buffer
         JSR   COUT                     ;Print it on the screen
         LDA   #TargDrv                 ;Prompt for drive
         LDY   #>TargDrv
         JSR   STROUT
LDrive   JSR   RDKEY                    ;Get a keypress
         CMP   #$B1                     ;Drive #1?
         BEQ   LConvert
         CMP   #$B2                     ;Drive #2?
         BNE   LDrive
LConvert STA   Buffer+1                 ;Store DRIVE number in Buffer+1
         JSR   COUT                     ;Print it on the screen
         JSR   DOTWO                    ;Print two carriage returns
         LDA   Buffer                   ;Fetch the SLOT number
         AND   #$0F                     ;Mask off the upper 4 bits
         rol   a                        ;Move lower 4 bits to upper 4 bits
         rol   a
         rol   a
         rol   a
         STA   Slot                     ;Store result in FORMAT slot
         tax
         LDA   Buffer+1                 ;Fetch the DRIVE number
         CMP   #$B1                     ;Does Slot need conditioning?
         Beq   Jump5                    ;Nope
Jump1    LDA   Slot                     ;Fetch FORMAT slot
         ORA   #$80                     ;Set MSB to indicate drive #2
         sta   Slot
         tax
Jump5    ldy   DevCnt                   ;Load how many devices
FLoop    lda   DevList,y                ; since this isn't a sequential
         sta   ListSlot                 ; list then must go through each one
         and   #$F0                     ; must also store what is there for later
         cmp   Slot
         beq   ItIsNum
         dey
         bpl   FLoop
         jmp   NoUnit                   ;Used to be bmi
ItIsNum  anop
         txa                            ;Make the slot the indexed register
         lsr   a                        ; for getting device drive controller
         lsr   a
         lsr   a
         tay
         lda   DevAdr,y                 ;Get it
         sta   Addr                     ; and save it
         lda   DevAdr+1,y
         sta   Addr+1
         tay
         and   #$F0                     ;Next see if it begins with a C0
         cmp   #$C0
         beq   YesSmart                 ; and if it does is a very smart device
         txa
         cmp   #$B0                     ;If it isn't smart test for /Ram
         beq   YesRam3
         clc
         ror   a
         ror   a
         ror   a
         ror   a
         and   #$07
         ora   #$C0
         sta   Addr+1                   ;if it isn't either then treat it as
         jmp   YesSmart1                ; smart and save what bank it is in.

YesRam3  anop
         lda   Addr+1                   ;If you think it is a /Ram then check
         cmp   #$FF                     ; the bits that tell you so.
         beq   Loop7                    ; Does the Address pointer start with FF
         jmp   NoUnit
Loop7    lda   Addr                     ; And end with 00
         cmp   #$00
         beq   Loop8
         jmp   NoUnit
Loop8    lda   ListSlot
         and   #$F3
         cmp   #$B3
         beq   Loop9
         jmp   NoUnit
Loop9    LDA   #ItIsRam3                ;Tell the preson that you think it is a
         LDY   #>ItIsRam3               ; /Ram and if they want to continue
         JSR   STROUT
         jsr   GetYN
         Bne   Jump2
         jsr   OldName
         jsr   Ram3Form
Jump2    jmp   AGain
YesSmart anop
         tya
         and   #$0F
         rol   a                        ;Move lower 4 bits to upper 4 bits
         rol   a
         rol   a
         rol   a
         STA   Slot                     ;Store result in FORMAT slot
YesSmart1 anop
         lda   Addr+1                   ;Check signiture bytes in the Cn page
         sta   Buffer+1                 ; for a smart device.
         lda   #$00
         sta   Buffer
         ldy   #$01
         lda   (Buffer),y
         cmp   #$20
         bne   NoUnit
         ldy   #$03
         lda   (Buffer),y
         bne   NoUnit
         ldy   #$05
         lda   (Buffer),y
         cmp   #$03
         bne   NoUnit
         ldy   #$FF
         lda   (Buffer),y
         cmp   #$00                     ;Apples DiskII
         beq   DiskII
         cmp   #$FF                     ;Wrong DiskII
         beq   NoUnit                   ; must be a smart device.
         ldy   #$07                     ;Test last signiture byte for the
         lda   (Buffer),y               ; Protocol Converter.
         cmp   #$00
         bne   NoUnit                   ;It isn't so its no device I know.
         LDA   #ItIsSmart               ;Tell them you think it is a SmartPort
         LDY   #>ItIsSmart              ; device. and ask if they want to Format.
         JSR   STROUT
         jsr   GetYN
         Bne   Jump3
         jsr   OldName                  ;Show old name and ask if proper Disk
         jsr   LName                    ;Get New name
         jsr   SmartForm                ;Jump too routine to format Smart Drive
         lda   ListSlot
         and   #$F0
         sta   Slot
         jsr   CodeWr                   ;Jump to routine to produce Bit map
         JMP   CATALOG                  ;Write Directory information to the disk
Jump3    jmp   Again


NoUnit   LDA   #UnitNone                ;Prompt to continue or not Because
         LDY   #>UnitNone               ;There is no unit number like that
         JSR   STROUT
         jsr   GetYN
         Bne   Jump4
         jmp   Reentry
Jump4    jmp   MExit

DiskII   anop
         LDA   #ItsaII                  ;Tell them you think it is a DiskII
         LDY   #>ItsaII
         JSR   STROUT
         jsr   GetYN
         Bne   Jump3
         LDA   #$18                     ;Set VOLblks to 280 ($118)
         LDX   #$01                     ; Just setting default settings
         ldy   #$00
         STA   VOLblks
         STX   VOLblks+1
         STY   VOLblks+2
         jsr   OldName                  ;Prompt for proper disk.
         jsr   LName                    ;Get new name
         jmp   DIIForm                  ;Format DiskII

LName    LDA   #VolName
         LDY   #>VolName
         JSR STROUT
LRdname  LDA   #$0E                     ;Reset CH to 14
         STA   CH
         LDX   #$00
         BEQ   LInput                   ;Always taken
LBackup  CPX   #0                       ;Handle backspaces
         BEQ   LRdname
         DEX
         LDA   #$88                     ;<--
         JSR   COUT
LInput   JSR   RDKEY                    ;Get a keypress
         CMP   #$88                     ;Backspace?
         BEQ   LBackup
         CMP   #$FF                     ;Delete?
         BEQ   LBackup
         CMP   #$8D                     ;C/R is end of input
         BEQ   LFormat
         CMP   #$AE                     ;(periods are ok...)
         BEQ   LStore
         CMP   #$B0                     ;Less than '0'?
         BCC   LInput
         CMP   #$BA                     ;Less than ':'?
         BCC   LStore                   ;Valid. Store the keypress
         AND   #$DF                     ;Force any lower case to upper case
         CMP   #$C0                     ;Less than 'A'?
         BEQ   LInput
         CMP   #$DB                     ;Greater than 'Z'?
         BCS   LInput
LStore   JSR   COUT                     ;Print keypress on the screen
         AND   #$7F                     ;Clear MSB
         STA   VOLnam,X                 ;Store character in VOLnam
         INX
         CPX   #$0E                     ;Have 15 characters been entered?
         BCC   LInput
LFormat  TXA                            ;See if default VOLUME_NAME was taken
         BNE   LSetLEN
WLoop    LDA   Blank,X                  ;Transfer 'BLANK' to VOLnam
         AND   #$7F                     ;Clear MSB
         STA   VOLnam,X
         INX
         CPX   #$05                     ;End of transfer?
         BCC   WLoop
         LDA   #$13                     ;Reset CH to 19
         STA   CH
LSetLEN  JSR   CLRLN                    ;Erase the rest of the line
         CLC
         TXA                            ;Add $F0 to Volume Name length
         ADC   #$F0                     ;Create STORAGE_TYPE, NAME_LENGTH byte
         STA   VOLlen
         rts

DIIForm  JSR   FORMAT                   ;Format the disk
         jsr   CodeWr                   ;Form Bitmap
         LDA   #Verif                   ;Ask if you want to Verify the Disk
         LDY   #>Verif
         JSR   STROUT
         JSR   GetYN                    ;Get a Yes or No answer to 'Verify?'
         Bne   BIIForm1
         jmp   VERIFY                   ;Answer was yes...
BIIForm1 JMP   CATALOG                  ;Write Directory information to the disk

CodeWr   LDA   #$81                     ;Set Opcode to WRITE
         STA   Opcode
**********************************
*                                *
* Write Block0 to disk           *
*                                *
**********************************
AskBlk0  LDA   #BootCode                ;Set MLIbuf to BootCode
         LDY   #>BootCode
         STA   MLIbuf
         STY   MLIbuf+1
         LDA   #$00                     ;Set MLIblk to 0
         STA   MLIblk
         STA   MLIblk+1
         JSR   CallMLI                  ;Write block #0 to target disk
**************************************
*                                    *
* Fill buffer $6800-$69FF with zeros *
* and prepare BitMap and Link blocks *
* for writing to disk                *
*                                    *
**************************************
Fill     anop
         lda   #$05                     ;Block 5 on Disk
         sta   MLIBlk
         LDA   #$00                     ;Set Buffer, MLIbuf to $6800
         LDX   #$68
         STA   MLIbuf
         STA   Buffer
         STX   MLIbuf+1
         STX   Buffer+1
         TAY                            ;Fill $6800-$69FF with zeros
         LDX   #$01                     ;Fill 2 pages of 256 bytes
LZero    STA   (Buffer),Y
         INY
         BNE   LZero
         INC   Buffer+1
         DEX
         BPL   LZero
         LDA   #$68                     ;Reset Buffer to $6800
         STA   Buffer+1
         LDA   #$05                     ;Length of DirTbl
         STA   Count
LLink    LDX   Count
         LDA   DirTbl,X                 ;Move Directory Link values into Buffer
         STA   $6802                    ;Store next Directory block #
         DEX
         LDA   DirTbl,X                 ;Fetch another # from DirTbl
         STA   $6800                    ;Store previous Directory block #
         DEX
         STX   Count
         JSR   CallMLI                  ;Write Directory Link values to disk
LDec     DEC   MLIblk                   ;Decrement MLI block number
         LDA   MLIblk                   ;See if MLIblk = 2
         CMP   #$02
         BNE   LLink                    ;Process another Link block
**********************************
*                                *
* Calculate BitMap Size and cndo *
*                                *
**********************************
BlkCount lda   VolBlks+1                ;Where # of blocks are stored
         ldx   VolBlks
         ldy   VolBlks+2                ;Can't deal with block devices this big
         bne   Jump10
         stx   Count+1                  ;Devide the # of blocks by 8 for bitmap
         lsr   a                        ; calculation
         ror   Count+1
         lsr   a
         ror   Count+1
         lsr   a
         ror   Count+1
         sta   Count+2
         jmp   BitMapCode
Jump10   pla                            ;Remove the address that the routine
         pla                            ; would have returned to
         lda   #$4D                     ;Make error Block size to large
         jmp   Died
BitMapCode anop
         lda   #%00000001               ;Clear first 7 blocks
         sta   (Buffer),y
         ldy   Count+1                  ;Original low block count value
         bne   jump11                   ;if it is 0 then make FF
         dey                            ;Make FF
         dec   Count+2                  ;Make 256 blocks less one
         sty   Count+1                  ;Make FF new low block value
Jump11   anop
         ldx   Count+2                  ;High Block Value
         bne   Jump15                   ;If it isn't equal to 0 then branch
         ldy   Count+1
         jmp   Jump19

Jump15   anop
         lda   #$69                     ;Set the adress of the upper part of
         sta   Buffer+1                 ; Block in bitmap being created
         lda   #%11111111
         ldy   Count+1                  ;Using the low byte count
Jump20   dey
         sta   (Buffer),y               ;Store them
         beq   Jump17
         jmp   Jump20
Jump17   anop
         dey                            ;Fill in first part of block
         lda   #$68
         sta   Buffer+1
Jump19   lda   #%11111111
         dey
         sta   (Buffer),y
         cpy   #$01                     ;Except the first byte.
         beq   Jump18
         jmp   jump19
Jump18   rts


*************************************
*                                   *
* VERIFY - Verify each block on the *
* disk, and flag bad ones in BITMAP *
*                                   *
*************************************
Verify   anop

         LDA   #$80                     ;Set Opcode to $80 (READ)
         STA   Opcode
         LDA   #$60                     ;Change Error to an RTS instruction
         STA   Error
         LDA   #$00                     ;Reset MLIbuf to $1000
         LDX   #$10
         STA   MLIbuf
         STX   MLIbuf+1
         STA   Count                    ;Set Count and Pointer to 0
         STA   Pointer
         STA   LBad                     ;Set bad block counter to 0
         STA   MLIblk                   ;Reset MLIblk to 0
         STA   MLIblk+1
LRead    JSR   CallMLI                  ;Read a block
         BCS   LError                   ;Update BitMap if error occurs
LIncBlk  CLC                            ;Add 1 to MLIblk
         INC   MLIblk
         BNE   LCheck
         INC   MLIblk+1
LCheck   INC   Count                    ;Add 1 to BitMap counter
         LDA   Count                    ;If Count > 7 then add 1 to Pointer
         CMP   #$08
         BCC   MDone
         LDA   #$00                     ;Reset Count to 0
         STA   Count
         INC   Pointer                  ;Add 1 to Pointer offset value
MDone    LDX   MLIblk                   ;See if we've read 280 blocks yet
         LDA   MLIblk+1
         BEQ   LRead
         CPX   #$18                     ;Greater than $118 (280) blocks read?
         BEQ   LResult                  ;Finished. Display results of VERIFY
         BNE   LRead                    ;Go get another block
LError   LDX   Count                    ;Use Count as offset into MapTbl
         LDA   BitTbl,X                 ;Fetch value for bad block number
         LDY   Pointer                  ;Use Pointer as offset into Buffer
         AND   (Buffer),Y               ;Mask value against BitMap value
         STA   (Buffer),Y               ;Store new BitMap value in Buffer
         CLC
         DEC   VOLblks                  ;Decrement # of blocks available
         BNE   LIncBad
         DEC   VOLblks+1
LIncBad  INC   LBad                     ;Add 1 to # of bad blocks found
         JMP   LIncBlk                  ;Get the next block on the disk
LResult  LDA   #$EA                     ;Change Error back to a ANOP instruction
         STA   Error
         LDA   LBad                     ;Find out if there were any bad blocks
         BEQ   LGood
         JSR   HEXDEC                   ;Convert hex number into decimal
         LDX   #$00
BLoop    LDA   IN,X                     ;Print the decimal Gequivalent of LBad
         CMP   #$31                     ;Don't print zeros...
         BCC   MNext
         JSR   COUT
MNext    INX
         CPX   #$03                     ;End of number?
         BCC   BLoop
         LDA   #Bad
         LDY   #>Bad
         JSR   STROUT
         JMP   CATALOG                  ;Write BitMap and Links to the disk
LGood    LDA   #Good
         LDY   #>Good
         JSR   STROUT
         JMP   CATALOG
LBad     DS    1                        ;Number of bad blocks found
*************************************
*                                   *
* CATALOG - Build a Directory Track *
*                                   *
*************************************
Catalog  anop

         LDA   #$81                     ;Change Opcode to $81 (WRITE)
         STA   Opcode
         LDA   #$00                     ;Reset MLIbuf to $6800
         LDX   #$68
         STA   MLIbuf
         STX   MLIbuf+1
         LDX   #$06                     ;Write Buffer (BitMap) to block #6 on the disk
         STX   MLIblk
         STA   MLIblk+1
         JSR   Call2MLI                  ; Call for time and date
         jsr   MLI
         dc    i1'$82'
         dc    i2'0'
         lda   $BF90                    Get them and save them into the
         sta   Datime                   ; Directory to be written.
         lda   $BF91
         sta   Datime+1
         lda   $BF92
         sta   Datime+2
         lda   $BF93
         sta   Datime+3
         LDY   #$2A                     ;Move Block2 information to $6800
CLoop    LDA   Block2,Y
         STA   (Buffer),Y
         DEY
         BPL   CLoop
         LDA   #$02                     ;Write block #2 to the disk
         STA   MLIblk
         JSR   Call2MLI
Again    anop
         LDA   #Nuther                  ;Display 'Format another' string
         LDY   #>Nuther
         JSR   STROUT
         JSR   GetYN                    ;Get a Yes or No answer
         BNE   MExit                    ;Answer was No...
         JMP   Reentry                  ;Format another disk
MExit    JSR   DOTWO                    ;Two final carriage returns...
         lda   QSlot
         sta   Last
         ldx   Stack                    ;Just because I am human to and might
         txs                            ;have messed up also.
         JMP   WARMDOS                  ;Exit to BASIC
Call2MLI anop
         jsr   CallMLI
         rts
***************************
*                         *
* GetYN - Get a Yes or No *
* answer from the user    *
*                         *
***************************
GetYN    anop

         JSR   RDKEY                    ;Get a keypress
         AND   #$DF                     ;Mask lowercase
         CMP   #$D9                     ;is it a Y?
         BEQ   LShow
         LDA   #$BE                     ;Otherwise default to "No"
LShow    JSR   COUT                     ;Print char, Z flag contains status
         CMP   #$D9                     ;Condition flag
         RTS
*************************************
*                                   *
* CallMLI - Call the MLI Read/Write *
* routines to transfer blocks to or *
* from memory                       *
*                                   *
*************************************
CallMLI  anop

         JSR   MLI                      ;Call the ProDOS Machine Langauge Interface
Opcode   dc    i1'$81'                  ;Default MLI opcode = $81 (WRITE)
         dc    i2'Parms'
         BCS   Error
         RTS
Error    anop                           ;(this will be changed to RTS by VERIFY)
         pla
         pla
         pla
         pla
         JMP   Died
**************************************
*                                    *
* DOTWO - Print two carriage returns *
*                                    *
**************************************

DOTWO    anop
         LDA   #$8D                     ;(we don't need an explanation, do we?)
         JSR   COUT
         JSR   COUT
         RTS
***********************************
*                                 *
* HEXDEC - Convert HEX to decimal *
*                                 *
***********************************

HEXDEC   anop
         STA   IN+20                    ;Store number in Keyboard Input Buffer
         LDA   #$00
         STA   IN+21
         LDY   #$02                     ;Result will be three digits long
DLoop    LDX   #$11                     ;16 bits to process
         LDA   #$00
         CLC
LDivide  ROL   a
         CMP   #$0A                     ;Value > or = to 10?
         BCC   LPlus
         SBC   #$0A                     ;Subtract 10 from the value
LPlus    ROL   IN+20                    ;Shift values in IN+20, IN+21 one bit left
         ROL   IN+21
         DEX
         BNE   LDivide
         ORA   #$B0                     ;Convert value to high ASCII character
         STA   IN,Y                     ;Store it in the input buffer
         sta   NetNum,y
         DEY
         BPL DLoop
         RTS
***********************************
*                                 *
* FORMAT - Format the target disk *
*                                 *
***********************************

Format   anop
         php
         sei
         LDA   Slot                     ;Fetch target drive SLOTNUM value
         PHA                            ;Store it on the stack
         AND   #$70                     ;Mask off bit 7 and the lower 4 bits
         STA   SlotF                    ;Store result in FORMAT slot storage
         TAX                            ;Assume value of $60 (drive #1)
         PLA                            ;Retrieve value from the stack
         BPL   LDrive1                  ;If < $80 the disk is in drive #1
         INX                            ;Set X offset to $61 (drive #2)
LDrive1  LDA   Select,X                 ;Set softswitch for proper drive
         LDX   SlotF                    ;Set X offset to FORMAT slot/drive
         LDA   DiskON,X                 ;Turn the drive on
         LDA   ModeRD,X                 ;Set Mode softswitch to READ
         LDA   DiskRD,X                 ;Read a byte
         LDA   #$23                     ;Assume head is on track 35
         STA   TRKcur
         LDA   #$00                     ;Destination is track 0
         STA   TRKdes
         JSR   SEEK                     ;Move head to track 0
         LDX   SlotF                    ;Turn off all drive phases
         LDA   Step0,X
         LDA   Step2,X
         LDA   Step4,X
         LDA   Step6,X
         LDA   TRKbeg                   ;Move TRKbeg value (0) to Track
         STA   Track
         JSR   BUILD                    ;Build a track in memory at $6700

*******************************
*                             *
* WRITE - Write track to disk *
*                             *
*******************************
Write    Entry
         JSR   CALC                     ;Calculate new track/sector/checksum values
         JSR   TRANS                    ;Transfer track in memory to disk
         BCS   DiedII                   ;If carry set, something died
MInc     INC   Track                    ;Add 1 to Track value
         LDA   Track                    ;Is Track > ending track # (TRKend)?
         CMP   TRKend
         BEQ   LNext                    ;More tracks to FORMAT
         BCS   DONE                     ;Finished.  Exit FORMAT routine
LNext    STA   TRKdes                   ;Move next track to FORMAT to TRKdes
         JSR   SEEK                     ;Move head to that track
         JMP   WRITE                    ;Write another track
DONE     LDX   SlotF                    ;Turn the drive off
         LDA   DiskOFF,X
         plp
         RTS                            ;FORMAT is finished. Return to calling routine
DiedII   PHA                            ;Save MLI error code on the stack
         JSR   Done
         PLA                            ;Retrieve error code from the stack
         JMP   Died                     ;Prompt for another FORMAT...

**************************************
*                                    *
* Died - Something awful happened to *
* the disk or drive. Die a miserable *
* death...                           *
*                                    *
**************************************
Died     anop
         cmp   #$4D                     ;Save MLI error code on the stack
         beq   RangeError
         cmp   #$27
         beq   DriveOpen
         cmp   #$2F
         beq   DiskError
         cmp   #$2B
         beq   Protected
         jmp   NoDied
RangeError LDA   #TooLarge
         LDY   #>TooLarge
         jmp   DiedOut
DiskError LDA   #NoDisk
         LDY   #>NoDisk
         jmp   DiedOut
DriveOpen LDA   #Dead
         LDY   #>Dead
         jmp   DiedOut
Protected LDA   #Protect
         LDY   #>Protect
         jmp   DiedOut
NoDied   PHA                            ;Save MLI error code on the stack
         LDA   #UnRecog
         LDY   #>UnRecog
         JSR   STROUT
         PLA                            ;Retrieve error code from the stack
         JSR   PRBYTE                   ;Print the MLI error code
         jmp   Again
DiedOut  JSR   STROUT
         JMP   Again                    ;Prompt for another FORMAT...
************************************
*                                  *
* TRANS - Transfer track in memory *
* to target device                 *
*                                  *
************************************

Trans    anop
         LDA   #$00                     ;Set Buffer to $6700
         LDX   #$67
         STA   Buffer
         STX   Buffer+1
         LDY   #$32                     ;Set Y offset to 1st sync byte (max=50)
         LDX   SlotF                    ;Set X offset to FORMAT slot/drive
         SEC                            ;(assum the disk is write protected)
         LDA   DiskWR,X                 ;Write something to the disk
         LDA   ModeRD,X                 ;Reset Mode softswitch to READ
         BMI   LWRprot                  ;If > $7F then disk was write protected
         LDA   #$FF                     ;Write a sync byte to the disk
         STA   ModeWR,X
         CMP   DiskRD,X
         NOP                            ;(kill some time for WRITE sync...)
         JMP   LSync2
LSync1   EOR   #$80                     ;Set MSB, converting $7F to $FF (sync byte)
         NOP                            ;(kill time...)
         NOP
         JMP   MStore
LSync2   PHA                            ;(kill more time... [ sheesh! ])
         PLA
LSync3   LDA   (Buffer),Y               ;Fetch byte to WRITE to disk
         CMP   #$80                     ;Is it a sync byte? ($7F)
         BCC   LSync1                   ;Yep. Turn it into an $FF
         NOP
MStore   STA   DiskWR,X                 ;Write byte to the disk
         CMP   DiskRD,X                 ;Set Read softswitch
         INY                            ;Increment Y offset
         BNE   LSync2
         INC   Buffer+1                 ;Increment Buffer by 255
         BPL   LSync3                   ;If < $8000 get more FORMAT data
         LDA   ModeRD,X                 ;Restore Mode softswitch to READ
         LDA   DiskRD,X                 ;Restore Read softswitch to READ
         CLC
         RTS
LWRprot  CLC                            ;Disk is write protected! (Nerd!)
         JSR   DONE                     ;Turn the drive off
         lda   #$2B
         pla
         pla
         pla
         pla
         JMP   Died                     ;Prompt for another FORMAT...
************************************
*                                  *
* BUILD - Build GAP1 and 16 sector *
* images between $6700 and $8000   *
*                                  *
************************************
Build    anop

         LDA   #$10                     ;Set Buffer to $6710
         LDX   #$67
         STA   Buffer
         STX   Buffer+1
         LDY   #$00                     ;(Y offset always zero)
         LDX   #$F0                     ;Build GAP1 using $7F (sync byte)
         LDA   #$7F
         STA   LByte
         JSR   LFill                    ;Store sync bytes from $6710 to $6800
         LDA   #$10                     ;Set Count for 16 loops
         STA   Count
LImage   LDX   #$00                     ;Build a sector image in the Buffer area
ELoop    LDA   LAddr,X                  ;Store Address header, info & sync bytes
         BEQ   LInfo
         STA   (Buffer),Y
         JSR   LInc                     ;Add 1 to Buffer offset address
         INX
         BNE   ELoop
LInfo    LDX   #$AB                     ;Move 343 bytes into data area
         LDA   #$96                     ;(4&4 encoded version of hex $00)
         STA   LByte
         JSR   LFill
         LDX   #$AC
         JSR   LFill
         LDX   #$00
YLoop    LDA   LData,X                  ;Store Data Trailer and GAP3 sync bytes
         BEQ   LDecCnt
         STA   (Buffer),Y
         JSR   LInc
         INX
         BNE   YLoop
LDecCnt  CLC
         DEC   Count
         BNE   LImage
         RTS                            ;Return to write track to disk (WRITE)
LFill    LDA   LByte
         STA   (Buffer),Y               ;Move A register to Buffer area
         JSR   LInc                     ;Add 1 to Buffer offset address
         DEX
         BNE   LFill
         RTS
LInc     CLC
         INC   Buffer                   ;Add 1 to Buffer address vector
         BNE   LDone
         INC   Buffer+1
LDone    RTS
***********************************
*                                 *
* CALC - Calculate Track, Sector, *
* and Checksum values of the next *
* track using 4&4 encoding        *
*                                 *
***********************************
Calc     anop

         LDA   #$03                     ;Set Buffer to $6803
         LDX   #$68
         STA   Buffer
         STX   Buffer+1
         LDA   #$00                     ;Set Sector to 0
         STA   Sector
ZLoop    LDY   #$00                     ;Reset Y offset to 0
         LDA   #$FE                     ;Set Volume # to 254 in 4&4 encoding
         JSR   LEncode
         LDA   Track                    ;Set Track, Sector to 4&4 encoding
         JSR   LEncode
         LDA   Sector
         JSR   LEncode
         LDA   #$FE                     ;Calculate the Checksum using 254
         EOR   Track
         EOR   Sector
         JSR   LEncode
         CLC                            ;Add 385 ($181) to Buffer address
         LDA   Buffer
         ADC   #$81
         STA   Buffer
         LDA   Buffer+1
         ADC   #$01
         STA   Buffer+1
         INC   Sector                   ;Add 1 to Sector value
         LDA   Sector                   ;If Sector > 16 then quit
         CMP   #$10
         BCC   ZLoop
         RTS                            ;Return to write track to disk (WRITE)
LEncode  PHA                            ;Put value on the stack
         LSR   a                        ;Shift everything right one bit
         ORA   #$AA                     ;OR it with $AA
         STA   (Buffer),Y               ;Store 4&4 result in Buffer area
         INY
         PLA                            ;Retrieve value from the stack
         ORA   #$AA                     ;OR it with $AA
         STA   (Buffer),Y               ;Store 4&4 result in Buffer area
         INY
         RTS
*************************************
*                                   *
* SEEK - Move head to desired track *
*                                   *
*************************************
Seek     anop

         LDA   #$00                     ;Set InOut flag to 0
         STA   LInOut
         LDA   TRKcur                   ;Fetch current track value
         SEC
         SBC   TRKdes                   ;Subtract destination track value
         BEQ   LExit                    ;If = 0 we're done
         BCS   LMove
         EOR   #$FF                     ;Convert resulting value to a positive number
         ADC   #$01
LMove    STA   Count                    ;Store track value in Count
         ROL   LInOut                   ;Condition InOut flag
         LSR   TRKcur                   ;Is track # odd or even?
         ROL   LInOut                   ;Store result in InOut
         ASL   LInOut                   ;Shift left for .Table offset
         LDY   LInOut
ALoop    LDA   LTable,Y                 ;Fetch motor phase to turn on
         JSR   PHASE                    ;Turn on stepper motor
         LDA   LTable+1,Y               ;Fetch next phase
         JSR   PHASE                    ;Turn on stepper motor
         TYA
         EOR   #$02                     ;Adjust Y offset into LTable
         TAY
         DEC   Count                    ;Subtract 1 from track count
         BNE   ALoop
         LDA   TRKdes                   ;Move current track location to TRKcur
         STA   TRKcur
LExit    RTS                            ;Return to calling routine
**********************************
*                                *
* PHASE - Turn the stepper motor *
* on and off to move the head    *
*                                *
**********************************

Phase    anop
         ORA   SlotF                    ;OR Slot value to PHASE
         TAX
         LDA   Step1,X                  ;PHASE on...
         LDA   #20                     ;20 ms. delay
         JSR   WAIT
         LDA   Step0,X                  ;PHASE off...
         RTS
**********************************
*                                *
* Format a Ram3 device.          *
*                                *
**********************************
Ram3Form anop
         php
         sei
         lda   #3                       ;Format Request number
         sta   $42
         Lda   slot                     ;Slot of /Ram
         sta   $43
         lda   #$00                     ;Buffer space if needed Low byte
         sta   $44
         lda   #$67                     ; and high byte
         sta   $45

         lda   $C08B                    ;Read and write Ram, using Bank 1
         lda   $C08B

         jsr   Ram3Dri
         bit   $C082                    ;Read ROM, use Bank 2(Put back on line)
         bcs   Ram3Err
         plp
         rts

Ram3Dri  jmp   (Addr)
Ram3Err  anop
         tax
         plp
         pla
         pla
         txa
         jmp   Died
**********************************
*                                *
* Format a SmartPort Device      *
*                                *
**********************************
SmartForm anop
         Php
         sei
         lda   #0                       ;Request Protocol converter for a Status
         sta   $42
         Lda   ListSlot                 ;Give it the ProDOS Slot number
         and   #$F0
         sta   $43
         lda   #$00                     ;Give it a buffer may not be needed by
         sta   $44                      ; give it to it anyways
         lda   #$68
         sta   $45
         lda   #$03                     ;The Blocks of Device
         sta   $46
         jsr   SmartDri
         txa                            ;Low in X register
         STA   VOLblks                  ; Save it
         tya                            ; High in Y register
         STa   VOLblks+1                ; Save it
         LDa   #$00
         STa   VOLblks+2
         lda   #3                       ;Give Protocol Converter a Format Request
         sta   $42                      ;Give it Slot number
         Lda   ListSlot
         and   #$F0
         sta   $43
         lda   #$00                     ;Give a buffer which probably won't be
         sta   $44                      ; used.
         lda   #$68
         sta   $45

         jsr   SmartDri
         bcs   SmartErr
         plp
         rts

SmartDri jmp   (Addr)
SmartErr anop
         tax
         plp
         pla
         pla
         txa
         jmp   Died

**********************************
*                                *
* Is there an old name?          *
*                                *
**********************************
OldName  anop
         lda   ListSlot
         and   #$F0
         STA   Info+1
         jsr   MLI
         dc    i1'$C5'
         dc    i2'Info'
         lda   VolLen
         and   #$0F
         bne   OldName1
         lda   VolLen+1
         cmp   #$28
         bne   OldError
         pla
         pla
         jmp   NoUnit
OldName1 sta   VolLen
         LDA   #TheOld1
         LDY   #>TheOld1
         JSR   STROUT
         lda   #VolLen                  ;Get Name Length
         ldy   #>VolLen
         jsr   Strout                   ;Print old name
         LDA   #TheOld2
         LDY   #>TheOld2
         JSR   STROUT
         jsr   GetYN
         beq   OldError
         pla
         pla
         jmp   Again
OldError anop
         rts

*************************
*                       *
* Variable Storage Area *
*                       *
*************************

Info     anop
         dc    i1'$02'
         ds    1
         dc    i2'VolLen'
Parms    dc    i1'$03'                  ;Parameter count = 3
Slot     dc    i1'$60'                  ;Default to S6,D1
MLIbuf   dc    i2'BootCode'             ;Default buffer address
MLIblk   dc    i2'$0000'                ;Default block number of 0
QSlot    ds    1                        ;Quit Slot number
ListSlot ds    1                        ;Saving the slot total from the list
Addr     ds    2
NetParms dc    i1'$00'
         dc    H'2F'                    ;Command for FIListSessions
         dc    i2'$0000'                ;Appletalk Result Code returned here
         dc    i2'$100'                 ;Length of string
         dc    i2'$6700'                ;Buffer low word
         dc    i2'$0000'                ;Buffer High word
NetDev   dc    i1'$00'                  ;Number of entries returned here
LByte    ds    1                        ;Storage for byte value used in Fill
LAddr    dc    H'D5 AA 96'              ;Address header
         dc    8i1'$AA'                 ;Volume #, Track, Sector, Checksum
         dc    H'DE AA EB'              ;Address trailer
         dc    6i1'$7F'                 ;GAP2 sync bytes
         dc    H'D5 AA AD'              ;Buffer header
         dc    H'00'                    ;End of Address information
LData    dc    H'DE AA EB'              ;Data trailer
         dc    16i1'$7F'                ;GAP3 sync bytes
         dc    H'00'                    ;End of Data information

LInOut   DS    1                        ;Inward/Outward phase for stepper motor
LTable   dc    H'02040600'              ;Phases for moving head inward
         dc    H'06040200'              ;   |    |    |      |  outward

TargSlt  dc    H'8D8D'
         dc    C'Format disk in SLOT ',H'00'
TargDrv  dc    C' DRIVE ',H'00'
VolName  dc    H'8D',C'Volume name: /'
Blank    dc    C'BLANK'
         dc    C'__________',H'00'
Verif    dc    H'8D 8D',C'Certify disk and mark any bad blocks in '
         dc    C'the Volume BitMap as unusable? (Y/N): ',H'00'
TheOld1  dc    H'8D',C'Do You Want To Write Over',H'8D A0 AF 00'
TheOld2  dc    H'A0 BF',C' (Y/N)',H'00'
UnRecog  dc    H'8D',C'Unrecognisable ERROR = ',H'00'
Dead     dc    H'8D',C'-- Check disk or drive door --',H'00'
Protect  dc    H'8D',C'Disk is write protected, you bozo!',H'00'
Bad      dc    H'8D 8D',C' bad block(s) marked',H'00'
Good     dc    H'8D 8D',C'Disk is error-free',H'00'
NoDisk   dc    H'8D',C'No Disk in the drive',H'00'
Nuther   dc    H'8D 8D',C'Format another disk? (Y/N): ',H'00'
TooLarge dc    H'8D',C'Unit Size Is To Large For this Program',H'00'
UnitNone dc    H'8D',C'No Unit in that slot and drive',H'8D'
         dc    C'Format another disk? (Y/N): ',H'00'
ItIsRam3 dc    H'8D',C'This is a Ram3 disk',H'8D'
         dc    C'Continue with Format? (Y/N): ',H'00'
ItsaII   dc    H'8D',C'This is a Disk II',H'8D'
         dc    C'Continue with Format? (Y/N): ',H'00'
ItIsSmart dc   H'8D',C'This is a SmartPort device',H'8D'
         dc    C'Continue with Format? (Y/N): ',H'00'
Appletalk anop
         dc    H'8D',C'Number of Appletalk devices is = '
NetNum   dc    H'00 00 00 8D',C'AppleTalk is installed this Program may',H'8D'
         dc    C'not work properly do you want to',H'8D'
         dc    C'Continue (Y/N)',H'00'
Block2   dc    H'00 00 03 00'
VOLlen   DS    1                        ;$F0 + length of Volume Name
VOLnam   DS    15           28          ;Volume Name, Reserved, Creation, Version
Reserved ds    6
UpLowCase dc   H'00 00'
Datime   ds    4
Version  dc    H'01'
         dc    H'00 C3 27 0D'
         dc    H'00 00 06 00'
VOLblks  ds    3                        ;Number of blocks available
DirTbl   dc    H'02 04 03'              ;Link list for directory blocks
         dc    H'05 04 00'
BitTbl   dc    B'01111111'              ;BitMap mask for bad blocks
         dc    B'10111111'
         dc    B'11011111'
         dc    B'11101111'
         dc    B'11110111'
         dc    B'11111011'
         dc    B'11111101'
         dc    B'11111110'
Stack    ds    1                        ;Entrance stack pointer
Count    DS    3                        ;General purpose counter/storage byte
Pointer  DS    2                        ;Storage for track count (8 blocks/track)
Track    DS    2                        ;Track number being FORMATted
Sector   DS    2                        ;Current sector number (max=16)
SlotF    DS    2                        ;Slot/Drive of device to FORMAT
TRKcur   DS    2                        ;Current track position
TRKdes   DS    2                        ;Destination track position
TRKbeg   dc    i1'00'                   ;Starting track number
TRKend   dc    i1'35'                   ;Ending track number
BootCode anop
         dc    H'0138B0034C32A18643C903088A29704A4A4A4A09C08549A0'
         dc    H'FF844828C8B148D03AB00EA9038D0008E63DA54948A95B48'
         dc    H'6085408548A063B148999409C8C0EBD0F6A206BC1D09BD24'
         dc    H'0999F209BD2B099D7F0ACA10EEA9098549A986A000C9F9B0'
         dc    H'2F85488460844A844C844E8447C88442C88446A90C856185'
         dc    H'4B201209B068E661E661E646A546C90690EFAD000C0D010C'
         dc    H'D06DA904D002A54A186D230CA8900DE64BA54B4AB006C90A'
         dc    H'F055A004844AAD0209290FA8B14AD90209D0DB8810F629F0'
         dc    H'C920D03BA010B14AC9FFD033C8B14A8546C8B14A8547A900'
         dc    H'854AA01E844B8461C8844D201209B017E661E661A44EE64E'
         dc    H'B14A8546B14C8547114AD0E74C00204C3F092650524F444F'
         dc    H'53202020202020202020A5608544A56185456C4800081E24'
         dc    H'3F454776F4D7D1B64BB4ACA62B18604CBC09A99F48A9FF48'
         dc    H'A901A2004C79F42058FCA01CB9500999AE058810F74C4D09'
         dc    H'AAAAAAA0D5CEC1C2CCC5A0D4CFA0CCCFC1C4A0D0D2CFC4CF'
         dc    H'D3A0AAAAAAA55329032A052BAABD80C0A92CA211CAD0FDE9'
         dc    H'01D0F7A62B60A5462907C9042903080A282A853DA5474AA5'
         dc    H'466A4A4A85410A8551A5458527A62BBD89C020BC09E627E6'
         dc    H'3DE63DB00320BC09BC88C060A5400A8553A9008554A55385'
         dc    H'5038E551F014B004E6539002C65338206D09A55018206F09'
         dc    H'D0E3A07F8452082838C652F0CE180888F0F5BD8CC010FB00'
         dc    H'0000000000000000'
         End
