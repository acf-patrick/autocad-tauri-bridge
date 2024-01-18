On Error Resume Next

Set gcad = GetObject(, "Acad.Application.22")
Set gcadDoc = gcad.ActiveDocument
gcadDoc.Application.WindowState = 3

cCmd = "TAURI"
gcad.ActiveDocument.SendCommand(cCmd & Chr(13))
