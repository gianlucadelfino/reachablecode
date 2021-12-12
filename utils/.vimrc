 " Pathogen
 call pathogen#infect()
 filetype off
 syntax on
 filetype plugin indent on


autocmd VimEnter * NERDTree
autocmd VimEnter * wincmd p

autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTree") && b:NERDTree.isTabTree()) | q | endif

let g:NERDTreeMouseMode=3

set mouse+=a
set number
"set ttymouse=xterm2

set colorcolumn=80
":hi ColorColumn guigb=#FFFFFF ctermbg=grey

"set list
"set listchars=trail~

"show tabs as 2 spaces
set tabstop=2

"when indenting with > use 2 spaces
set shiftwidth=2

"insert 2 spaces on tab
set expandtab

nnoremap <silent> <C-Right> <c-w>l
nnoremap <silent> <C-Left> <c-w>h
nnoremap <silent> <C-Up> <c-w>k
nnoremap <silent> <C-Down> <c-w>j


imap <Insert> <Nop>
inoremap <S-Insert> <Insert>
