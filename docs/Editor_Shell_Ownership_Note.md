# Editor Shell Ownership Note

Last updated: July 24, 2026

## Decision

Trong tai lieu kien truc, team dung `EditorShell` de chi `module` shell cap editor.

`EditorShell` la lop `Qt Widgets` hien tai dang implement `EditorShell`.

Khong dung `EditorEditorShell` lam ten kien truc chinh trong docs, vi ten do qua gan voi adapter hien tai va lam mo ownership cap `module`.

## Ownership

`EditorShell` thuoc `Core/Application`.

`EditorShell` dong vai tro:

* application shell
* workspace shell
* composition root

`EditorShellContexts` cung thuoc `Core/Application` vi no la adapter gom context va dependency composition cho editor shell.

## What Stays In EditorShell

`EditorShell` nen giu:

* menu / toolbar / dock / panel assembly
* Qt signal hookup
* composition cua controller va context factory
* shell-level result application
* shell-level status bar / script log feedback

## What Does Not Belong To EditorShell

`EditorShell` khong phai feature module host.

Vi vay, cac policy va workflow sau khong nen co canonical home trong `EditorShell`:

* `Animation` playback / key editing policy
* `Rigging` mutation policy
* `Scene` creation / selection / hierarchy policy
* `Rendering` camera / display behavior beyond shell wiring

Neu mot hanh vi co the song sau mot seam sau hon, `EditorShell` chi nen wire intent vao seam do.

## File Map

Current `EditorShell` ownership trong repo:

* [apps/editor/main.cpp](</E:/Animation Software/apps/editor/main.cpp>)
* [src/core/app/EditorShell.h](</E:/Animation Software/src/core/app/EditorShell.h>)
* [src/core/app/EditorShell.cpp](</E:/Animation Software/src/core/app/EditorShell.cpp>)
* [src/core/app/EditorShellContexts.h](</E:/Animation Software/src/core/app/EditorShellContexts.h>)
* [src/core/app/EditorShellContexts.cpp](</E:/Animation Software/src/core/app/EditorShellContexts.cpp>)

## Naming Rule

Trong docs:

* dung `EditorShell` khi noi ve `module` va ownership
* dung `EditorShell` khi noi ve lop `Qt` cu the

Dieu nay giup interface cua tai lieu sau hon: doi adapter sau nay khong doi ten `module`.
