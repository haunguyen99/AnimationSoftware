# Editor Shell Ownership Note

Last updated: July 24, 2026

## Decision

Trong tai lieu kien truc, team dung `EditorShell` de chi `module` shell cap editor.

`MainWindow` la lop `Qt Widgets` hien tai dang implement `EditorShell`.

Khong dung `EditorMainWindow` lam ten kien truc chinh trong docs, vi ten do qua gan voi adapter hien tai va lam mo ownership cap `module`.

## Ownership

`MainWindow` thuoc `Core/Application`.

`MainWindow` dong vai tro:

* application shell
* workspace shell
* composition root

`MainWindowContexts` cung thuoc `Core/Application` vi no la adapter gom context va dependency composition cho editor shell.

## What Stays In MainWindow

`MainWindow` nen giu:

* menu / toolbar / dock / panel assembly
* Qt signal hookup
* composition cua controller va context factory
* shell-level result application
* shell-level status bar / script log feedback

## What Does Not Belong To MainWindow

`MainWindow` khong phai feature module host.

Vi vay, cac policy va workflow sau khong nen co canonical home trong `MainWindow`:

* `Animation` playback / key editing policy
* `Rigging` mutation policy
* `Scene` creation / selection / hierarchy policy
* `Rendering` camera / display behavior beyond shell wiring

Neu mot hanh vi co the song sau mot seam sau hon, `MainWindow` chi nen wire intent vao seam do.

## File Map

Current `EditorShell` ownership trong repo:

* [apps/editor/main.cpp](</E:/Animation Software/apps/editor/main.cpp>)
* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [apps/editor/include/MainWindowContexts.h](</E:/Animation Software/apps/editor/include/MainWindowContexts.h>)
* [apps/editor/src/MainWindowContexts.cpp](</E:/Animation Software/apps/editor/src/MainWindowContexts.cpp>)

## Naming Rule

Trong docs:

* dung `EditorShell` khi noi ve `module` va ownership
* dung `MainWindow` khi noi ve lop `Qt` cu the

Dieu nay giup interface cua tai lieu sau hon: doi adapter sau nay khong doi ten `module`.
