command Build :!cmd.exe /c "shell.bat & build.bat"<CR>
command Run :!"build/Arc03.exe"

cnoreabbrev b Build
cnoreabbrev r Run

set wildignore+=*/build/*

ab sType sType = VK_STRUCTURE_TYPE_
