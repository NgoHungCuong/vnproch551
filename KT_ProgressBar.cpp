#include "KT_ProgressBar.h"

void KT_ProgressBar::SetNum(uint32_t u32Num)
{
    this->u32Num = u32Num;
}
void KT_ProgressBar::SetSig(char cSig)
{
    this->cSig = cSig;
}
void KT_ProgressBar::SetMax(uint32_t u32Max)
{
    this->u32Max = u32Max;
}
void KT_ProgressBar::SetPos(uint32_t u32Pos)
{
    this->u32Pos = u32Pos;
}

void KT_ProgressBar::Display(void)
{
    /* erase current line */
    printf("%c[2K\r", 27);
    /* display current state */
    uint32_t i;
    uint32_t n;
    n = u32Pos * u32Num / u32Max;
    for (i = 0; i < n; ++i) {
        printf("%c", cSig);
    }
    fflush(stdout);
}
