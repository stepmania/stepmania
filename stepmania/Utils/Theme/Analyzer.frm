VERSION 5.00
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Dirty theme metrics analyzer"
   ClientHeight    =   5895
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   8190
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5895
   ScaleWidth      =   8190
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdRemoveCommonMetrics 
      Caption         =   "Remove common metrics && reprint screens (must find common metrics first)"
      Height          =   495
      Left            =   4920
      TabIndex        =   9
      Top             =   2040
      Width           =   3135
   End
   Begin VB.TextBox txtOutput 
      Height          =   3375
      Left            =   2400
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   8
      Top             =   2520
      Width           =   5775
   End
   Begin VB.CommandButton Command1 
      Caption         =   "<R<"
      Height          =   855
      Left            =   5160
      TabIndex        =   7
      Top             =   1200
      Width           =   255
   End
   Begin VB.CommandButton cmdFindCommon 
      Caption         =   "Find Common Metrics"
      Height          =   255
      Left            =   2400
      TabIndex        =   6
      Top             =   2160
      Width           =   2055
   End
   Begin VB.ListBox lstToDoScreens 
      Height          =   2010
      Left            =   5520
      TabIndex        =   5
      Top             =   0
      Width           =   2655
   End
   Begin VB.CommandButton cmdAdd 
      Caption         =   ">ADD>"
      Height          =   1095
      Left            =   5160
      TabIndex        =   4
      Top             =   0
      Width           =   255
   End
   Begin VB.ListBox lstScreens 
      Height          =   2010
      Left            =   2400
      TabIndex        =   3
      Top             =   0
      Width           =   2655
   End
   Begin VB.DriveListBox Drive 
      Height          =   315
      Left            =   0
      TabIndex        =   2
      Top             =   4560
      Width           =   2295
   End
   Begin VB.FileListBox File 
      Height          =   2040
      Left            =   0
      Pattern         =   "*.ini"
      TabIndex        =   1
      Top             =   0
      Width           =   2295
   End
   Begin VB.DirListBox Dir 
      Height          =   2340
      Left            =   0
      TabIndex        =   0
      Top             =   2160
      Width           =   2295
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim NumScreens As Integer
Dim NumMetrics(0 To 1000) As Integer
Dim Screens(0 To 1000, 0 To 5000) As String

Dim CommonLines(0 To 5000) As String
Dim NUMCOMMONS As Integer

Private Sub cmdAdd_Click()
    lstToDoScreens.AddItem (lstScreens.List(lstScreens.ListIndex))
End Sub

Private Sub cmdFindCommon_Click()
    If lstToDoScreens.ListCount < 2 Then Exit Sub
    
    NUMCOMMONS = 0

    txtOutput = ""
    For i = 0 To 5000
        CommonLines(i) = ""
    Next i
    
    If lstToDoScreens.ListCount < 2 Then Exit Sub
    
    For i = 0 To NumMetrics(Val(lstToDoScreens.List(0)))
        CommonLines(i) = Screens(Val(lstToDoScreens.List(0)) - 1, i)
    Next i
    NUMCOMMONS = NumMetrics(Val(lstToDoScreens.List(0)))
    
    For i = 2 To lstToDoScreens.ListCount
        For j = 0 To NUMCOMMONS
            foundmetric = False
            For k = 0 To NumMetrics(Val(lstToDoScreens.List(i - 1)) - 1)
                MetricX$ = Screens(Val(lstToDoScreens.List(i - 1)) - 1, k)
                If CommonLines(j) = MetricX$ Then
                    foundmetric = True
                    Exit For
                End If
            Next k
            If foundmetric = False Then
                CommonLines(j) = ""
            End If
        Next j
    Next i
    
    For i = 0 To NUMCOMMONS
        If CommonLines(i) <> "" Then
            txtOutput = txtOutput + CommonLines(i) + vbCrLf
        End If
        
    Next i
End Sub

Private Sub cmdRemoveCommonMetrics_Click()
    txtOutput = ""
    For i = 1 To lstToDoScreens.ListCount
        For j = 0 To NUMCOMMONS
            foundmetric = False
            For k = 0 To NumMetrics(Val(lstToDoScreens.List(i - 1)) - 1)
                MetricX$ = Screens(Val(lstToDoScreens.List(i - 1)) - 1, k)
                If CommonLines(j) = MetricX$ Then
                    foundmetric = True
                    Exit For
                End If
            Next k
            If foundmetric = True Then
                Screens(Val(lstToDoScreens.List(i - 1)) - 1, k) = ""
            End If
        Next j
    Next i
    
    For i = 1 To lstToDoScreens.ListCount
        For j = 0 To NumMetrics(Val(lstToDoScreens.List(i - 1)) - 1)
            MetricX$ = Screens(Val(lstToDoScreens.List(i - 1)) - 1, j)
            If MetricX$ <> "" Then
                txtOutput = txtOutput + MetricX$ + vbCrLf
            End If
        Next j
    Next i

End Sub

Private Sub Command1_Click()
    If lstToDoScreens.ListIndex < 0 Then Exit Sub
    lstToDoScreens.RemoveItem (lstToDoScreens.ListIndex)
End Sub

Private Sub Dir_Change()
    File = Dir
End Sub

Private Sub Drive_Change()
    Dir = Drive
    File = Drive
End Sub

Private Sub File_Click()
    NumScreens = 0
    For i = 0 To 1000
        NumMetrics(i) = 0
    Next i
    CurScreen$ = ""
    lstScreens.Clear
    
    Open Dir + "\" + File For Binary As #1
    t = 1
    k$ = String$(2, "  ")
    xLine$ = ""
    
    While Not EOF(1)
        Get #1, t, k$
        While k$ <> vbCr + vbLf And Not EOF(1)
            t = t + 1
            xLine$ = xLine$ + Mid$(k$, 1, 1)
            Get #1, t, k$
        Wend
        t = t + 2
        If Mid(xLine$, 1, 1) = "[" Then
            Screens(NumScreens, 0) = Str(NumScreens + 1) + ":" + xLine$
            NumMetrics(NumScreens) = 1
            NumScreens = NumScreens + 1
            lstScreens.AddItem (Str(NumScreens) + ":" + xLine$)
        Else
            Screens(NumScreens - 1, NumMetrics(NumScreens - 1)) = xLine$
            NumMetrics(NumScreens - 1) = NumMetrics(NumScreens - 1) + 1
        End If
        xLine$ = ""
    Wend
    Close #1
End Sub

