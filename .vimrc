" Build debug
command Build :call term_sendkeys("console", "build.bat -d\<CR>")
" Build release
command BuildR :call term_sendkeys("console", "build.bat\<CR>")
command Run :call term_sendkeys("console", "build\\Arc03.exe\<CR>")
command CompileShaders :!cmd.exe /c "cd shaders & compile.bat"<CR>

cnoreabbrev b Build
cnoreabbrev br BuildR
cnoreabbrev r Run

set wildignore+=*/build/*

ab sType sType = VK_STRUCTURE_TYPE_

" Layout
set noequalalways
belowright call term_start("cmd.exe /k shell.bat", {'term_name':'console', 'term_rows':12, 'term_kill':'quit'})

" Close Vim when only console is left
autocmd bufenter * if (winnr("$") == 1 && bufwinnr("console") == 1) | q! | endif
