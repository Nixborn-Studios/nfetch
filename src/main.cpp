#include <iostream>
#include <vector>
#include <string>
#include <chafa.h>
#include <globals.hpp>
#include <utils.hpp>

int main()
{
    STBImageReturn* nixborn_img = NFETCH::LoadImage(nixborn_logo);
    NFETCH::PrintNixbornStyle(nixborn_img, 30, 15);

    delete nixborn_img;

    return 0;
}