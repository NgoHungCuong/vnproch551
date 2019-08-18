#ifndef KT_PROGRESS_BAR_H_
#define KT_PROGRESS_BAR_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

class KT_ProgressBar {
private:
    uint32_t u32Max = 100;
    char cSig = '.';
    uint32_t u32Num = 100;
    uint32_t u32Pos = 0;
public:
    void SetNum(uint32_t u32Num);
    void SetSig(char cSig);
    void SetMax(uint32_t u32Max);
    void SetPos(uint32_t u32Pos);
    void Display(void);
};

#endif
