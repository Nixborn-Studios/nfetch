#include <iostream>
#include <vector>
#include <string>
#ifdef HAVE_CHAFA
  #include <chafa.h>
#endif
#include <globals.hpp>
#include <utils.hpp>

int main()
{
    #ifdef HAVE_CHAFA
    STBImageReturn* nixborn_img = NFETCH::LoadImage(nixborn_logo.c_str());
    NFETCH::PrintNixbornStyle(nixborn_img, 30, 15);

    delete nixborn_img;
    #else
    std::cout << "This terminal doesn't support nfetch!";
    #endif

    return 0;
}