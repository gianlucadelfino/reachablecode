 " Pathogen
 call pathogen#infect()
 filetype off
 syntax on
 filetype plugin indent on


autocmd VimEnter * NERDTree

autocmd VimEnter * NERDTree
autocmd VimEnter * wincmd p

autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTree") && b:NERDTree.isTabTree()) | q | endif


set mouse=a
set number

nnoremap <silent> <C-Right> <c-w>l
nnoremap <silent> <C-Left> <c-w>h
nnoremap <silent> <C-Up> <c-w>k
nnoremap <silent> <C-Down> <c-w>j

