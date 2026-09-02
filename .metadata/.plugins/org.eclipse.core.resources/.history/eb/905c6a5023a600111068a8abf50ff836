#include "stm32f4xx.h"
//lấy nội dung của f411

int main(void)
{
    RCC->AHB1ENR |= (1 << 0); //RCC quyết định clock  AHB!ENR(là thanh ghi dùng để enable clovk trên AHB1)
                              // 1 << a dịch bit 1 sang bên trái a lần dấu OR để bit nào là 1 giữ nguyên nếu là 0 thì thay bằng 1

    GPIOA->MODER &= ~(3 << (5 * 2));      //GPIO port A mỗi pin sử dụng 2 bit ở đây sử dụng pin 5 tức là bit 10 và 11
                                          //~ để đảo bit 0-.1 1->0;  &= để set lại bit 10 và 11 thành 00
                                          //3 << 10 để sử dụng 2 bỉt 10 và 11 vì 3 nhị phân là 11
    GPIOA->MODER |=  (1 << (5 * 2));      //sau khi set bit 10 và 11 về 00 đẩy 1 lên bit 10 để thành 01 và trở thành trạng thái ENABLE

    while (1)
    {
        GPIOA->ODR  |=  (1 << 5);          //ODR chứa giá trị output của GPIO ở 2 thạng thái 0 là tắt và 1 là sáng

    }
}
