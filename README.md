# ReadMe



> 初期想法，是有一個介面，上面有按鈕，比方說Set紐和Color紐，後面帶color資料(例如0x33 0x00)。
> 按了之後，它會去執行
> **usiwr.exe -w=0x65 0x01 0x33 0x00**這樣的指令，去設定特定的HID周邊裝置的資料。
> 也可以透過Get紐和Color紐，執行**usiwr.exe -r=0x65 4** 。得到0x65 0x01 **0x33 0x00** 的資料。
>
> 
> 這些command table的細節可能之後會再詳細討論。只是先讓你看看可不可行。
>
> 會附上源碼，是因為或許您也不同想法，比方說您不想要呼叫這個外部的exe執行檔，你想把它的程式邏輯整合到
> 你自己開發的程式內也不一定。
>
> 如果您看過覺得可以接受，我們可在細談細節。謝謝您。

---

#### usirwexe

###### 		`編譯好的WIN10的執行檔和它所需要的QT dll files`

#### Batchfiles

###### 		`執行USIWR的批次檔`

#### USIWR

###### 		`原始碼，QT C++`

#### **USI_HID_Cmd_Table_SOP_181120_v03**

​	`Command Table(有Set和Get兩種，亦即Write和Read)。及Console mode下的執行範例。`



