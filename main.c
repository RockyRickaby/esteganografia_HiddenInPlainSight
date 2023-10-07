#include <stdio.h>
#include "lib_ppm.h"

#define HID_SIZE_MASK 0xC00
#define HID_BIT_MASK 0xC00000 // gotta bitshift it so we can take the correct bits from the image to be hidden
#define REM_BIT_MASK_RGB 0xFC // it's a good idea to "clean" the bits which will hide the other bits to be hidden so we avoid losing information about them

#define RECOVER_BIT 0x03
#define R_MASK 0xFF0000
#define G_MASK 0x00FF00
#define B_MASK 0x0000FF

//concatenates the pixel's rgb values into a single
//number and returns it so it becomes easier to put
//it into the other image's pixels
int concat_pixel_rgb(Pixel *pix) {
    return ((((pix->r << 8) | pix->g) << 8) | pix->b);
}

int recover(char *filename) {
    Img img, recovered;
    Img *p_img = &img,
        *p_rec = &recovered;

    if (read_ppm(filename, p_img)) {
        return 1;
    }

    int wid = 0, hei = 0, i = 0;
    for (i = 0; i < 2; i++) {
        wid = (wid | (p_img->pix[i].r & RECOVER_BIT)) << 2;
        wid = (wid | (p_img->pix[i].g & RECOVER_BIT)) << 2;
        wid = (wid | (p_img->pix[i].b & RECOVER_BIT)) << 2;
    }
    wid >>= 2;
    printf("Rec Width: %d\n", wid);
    for (i = 2; i < 4; i++) {
        hei = (hei | (p_img->pix[i].r & RECOVER_BIT)) << 2;
        hei = (hei | (p_img->pix[i].g & RECOVER_BIT)) << 2;
        hei = (hei | (p_img->pix[i].b & RECOVER_BIT)) << 2;
    }
    hei >>= 2;
    printf("Rec Height: %d\n", hei);
    if (new_ppm(p_rec, wid, hei)) {
        return 1;
    }
    int dec_len = wid * hei;
    int k = 4;
    for (i = 0; i < dec_len; i++) {
        int color = 0x0;
        for (int j = 0; j < 4; j++) {
            color = (color | (p_img->pix[k].r & RECOVER_BIT)) << 2;
            color = (color | (p_img->pix[k].g & RECOVER_BIT)) << 2;
            color = (color | (p_img->pix[k].b & RECOVER_BIT)) << 2;
            k++;
        }
        color >>= 2;
        //printf("%X ", color);
        p_rec->pix[i].r = ((color & R_MASK) >> 16);
        p_rec->pix[i].g = ((color & G_MASK) >> 8);
        p_rec->pix[i].b = (color & B_MASK);

    }
    write_ppm("recovered.ppm", p_rec);
    free_ppm(p_rec);
    free_ppm(p_img);
    return 0;

}

int hide(char *to_hide, char *where_to_hide, char *filename) {
    Img hide, holder;
    Img *p_hide = &hide,
        *p_holder = &holder;

    if (read_ppm(to_hide, p_hide)) {
        return 1;
    }
    if (read_ppm(where_to_hide, p_holder)) {
        return 1;
    }

    int hidden_len = p_hide->width * p_hide->height;
    int holder_len = p_holder->width * p_holder->height;

    if ((holder_len / 4) < hidden_len) {
        return 1;
    }
    int r = 0, i = 0;
    for (i = 0; i < 2; i++) {
        p_holder->pix[i].r = (p_holder->pix[i].r & REM_BIT_MASK_RGB) | ((p_hide->width & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
        p_holder->pix[i].g = (p_holder->pix[i].g & REM_BIT_MASK_RGB) | ((p_hide->width & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
        p_holder->pix[i].b = (p_holder->pix[i].b & REM_BIT_MASK_RGB) | ((p_hide->width & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
    }
    r = 0;
    // printf("%d\n", (p_holder->pix[2].r & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK)) >> (10)));
    // printf("%d\n", (p_holder->pix[2].g & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK >> 2)) >> (10 - 2)));
    // printf("%d\n", (p_holder->pix[2].g & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK >> 4)) >> (10 - 4)));
    for (i = 2; i < 4; i++) {
        p_holder->pix[i].r = (p_holder->pix[i].r & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
        p_holder->pix[i].g = (p_holder->pix[i].g & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
        p_holder->pix[i].b = (p_holder->pix[i].b & REM_BIT_MASK_RGB) | ((p_hide->height & (HID_SIZE_MASK >> r)) >> (10 - r));
        r += 2;
    }
    //hiding the image itself
    int k = 4;
    for (i = 0; i < hidden_len; i++) {
        int color = concat_pixel_rgb(&p_hide->pix[i]);
        //printf("%x ", color);
        r = 0;
        for (int j = 0; j < 4; j++) {
            p_holder->pix[k].r = (p_holder->pix[k].r & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
            r += 2;
            p_holder->pix[k].g = (p_holder->pix[k].g & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
            r += 2;
            p_holder->pix[k].b = (p_holder->pix[k].b & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
            r += 2;
            k++;
        }
    }
    if (write_ppm(filename, p_holder)) {
        return 1;
    }
    free_ppm(p_holder);
    free_ppm(p_hide);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc <= 1 || argc > 4) {
        printf("Usage: \"./hide <image_to_hide>.ppm <where_to_hide>.ppm <filename>.ppm\" for hiding an image or\n\"./hide <image>.ppm\" for recovering an image (without any quotes)");
        return 1;
    }
    if (argc == 2) {
        if (recover(argv[1])) {
            printf("It was not possible to recover the image.\n");
            return 1;
        }
        printf("Image recovered as recovered.ppm\n");
    } else if (argc == 4) {
        if (hide(argv[1], argv[2], argv[3])) {
            printf("It was not possible to hide the image.\n");
            return 1;
        }
        printf("Image %s hidden in %s and saved as %s", argv[1], argv[2], argv[3]);
    }
    return 0;
}

// int main()
// {
//     Img hidee, hidden, hid;
//     Img *to_hide_in = &hidee,
//         *to_be_hidden = &hidden,
//         *the_hidden = &hid;
//     int i = 0;

//     read_ppm("earth.ppm", to_hide_in);
//     read_ppm("frecl.ppm", to_be_hidden);

//     // printf("%X\n", concat_pixels(&to_be_hidden->pix[0]));
//     // printf("%X\n", concat_pixels(&to_be_hidden->pix[1]));
    

//     int hide_len = to_hide_in->width * to_hide_in->height;
//     int hidden_len = to_be_hidden->width * to_be_hidden->height;

//     if ((hide_len >> 2) < hidden_len) {
//         printf("A imagem a ser escondida não é pequena o suficiente para caber na outra imagem.\n");
//         return 1;
//     }
//     // printf("%X\n", concat_pixel_rgb(to_be_hidden->pix));

//     // for (int j = 0; j < 4; j++) {
//     //     printf("%X\n", concat_pixel_rgb(&to_hide_in->pix[j]));
//     // }

//     // int original_val = concat_pixel_rgb(to_hide_in->pix);
//     // int hidden_val = concat_pixel_rgb(to_be_hidden->pix);
    
//     int r = 0;
//     for (i = 0; i < 2; i++) {
//         to_hide_in->pix[i].r = (to_hide_in->pix[i].r & REM_BIT_MASK_RGB) | ((to_be_hidden->width & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//         to_hide_in->pix[i].g = (to_hide_in->pix[i].g & REM_BIT_MASK_RGB) | ((to_be_hidden->width & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//         to_hide_in->pix[i].b = (to_hide_in->pix[i].b & REM_BIT_MASK_RGB) | ((to_be_hidden->width & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//     }
//     r = 0;
//     // printf("%d\n", (to_hide_in->pix[2].r & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK)) >> (10)));
//     // printf("%d\n", (to_hide_in->pix[2].g & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK >> 2)) >> (10 - 2)));
//     // printf("%d\n", (to_hide_in->pix[2].g & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK >> 4)) >> (10 - 4)));
//     for (i = 2; i < 4; i++) {
//         to_hide_in->pix[i].r = (to_hide_in->pix[i].r & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//         to_hide_in->pix[i].g = (to_hide_in->pix[i].g & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//         to_hide_in->pix[i].b = (to_hide_in->pix[i].b & REM_BIT_MASK_RGB) | ((to_be_hidden->height & (HID_SIZE_MASK >> r)) >> (10 - r));
//         r += 2;
//     }
//     //hidding the image itself
//     int k = 4;
//     for (i = 0; i < hidden_len; i++) {
//         int color = concat_pixel_rgb(&to_be_hidden->pix[i]);
//         //printf("%x ", color);
//         r = 0;
//         for (int j = 0; j < 4; j++) {
//             to_hide_in->pix[k].r = (to_hide_in->pix[k].r & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
//             r += 2;
//             to_hide_in->pix[k].g = (to_hide_in->pix[k].g & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
//             r += 2;
//             to_hide_in->pix[k].b = (to_hide_in->pix[k].b & REM_BIT_MASK_RGB) | ((color & (HID_BIT_MASK >> r)) >> (22 - r));
//             r += 2;
//             k++;
//         }
//     }

    
//     // hidden_val = (original_val & REM_BIT_MASK) | ((hidden_val & (HID_BIT_MASK)) >> 22);
//     // // printf("\n%X\n", original_val);
//     // printf("\n%X\n", hidden_val);

//     // printf("\nOriginal %X\n", to_be_hidden->pix[0].r);
//     // printf("New %X\n", to_be_hidden->pix[0].r & REM_BIT_MASK_RGB);
//     printf("Height: %X, %d\n", to_be_hidden->height, to_be_hidden->height);
//     printf("Width: %X, %d\n", to_be_hidden->width, to_be_hidden->width);

//     write_ppm("test.ppm", to_hide_in);
//     // free_ppm(to_hide_in);
//     free_ppm(to_be_hidden);

//     //tryna recover the image, starting with its resolution
//     int wid = 0, hei = 0;
//     for (i = 0; i < 2; i++) {
//         wid = (wid | (to_hide_in->pix[i].r & RECOVER_BIT)) << 2;
//         wid = (wid | (to_hide_in->pix[i].g & RECOVER_BIT)) << 2;
//         wid = (wid | (to_hide_in->pix[i].b & RECOVER_BIT)) << 2;
//     }
//     wid >>= 2;
//     printf("Rec Width: %d\n", wid);
//     for (i = 2; i < 4; i++) {
//         hei = (hei | (to_hide_in->pix[i].r & RECOVER_BIT)) << 2;
//         hei = (hei | (to_hide_in->pix[i].g & RECOVER_BIT)) << 2;
//         hei = (hei | (to_hide_in->pix[i].b & RECOVER_BIT)) << 2;
//     }
//     hei >>= 2;
//     printf("Rec Height: %d\n", hei);

//     new_ppm(the_hidden, wid, hei);

//     // int color = 0;
//     // color = color | (to_hide_in->pix[4].r & RECOVER_BIT);
//     int dec_len = wid * hei;
//     k = 4;
//     for (i = 0; i < dec_len; i++) {
//         int color = 0x0;
//         for (int j = 0; j < 4; j++) {
//             color = (color | (to_hide_in->pix[k].r & RECOVER_BIT)) << 2;
//             color = (color | (to_hide_in->pix[k].g & RECOVER_BIT)) << 2;
//             color = (color | (to_hide_in->pix[k].b & RECOVER_BIT)) << 2;
//             k++;
//         }
//         color >>= 2;
//         //printf("%X ", color);
//         the_hidden->pix[i].r = ((color & R_MASK) >> 16);
//         the_hidden->pix[i].g = ((color & G_MASK) >> 8);
//         the_hidden->pix[i].b = (color & B_MASK);

//     }
    
//     write_ppm("rec.ppm", the_hidden);
//     free_ppm(the_hidden);
//     free_ppm(to_hide_in);
//     return 0;
// }
