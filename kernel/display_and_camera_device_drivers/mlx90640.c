#include "mlx90640.h"


uint8_t mlx90640_init(struct mlx90640_dev *device)
{
    unsigned char data[3];
    unsigned char reg_addr[2];

    reg_addr[0]=0x24; reg_addr[1]=0x07;

    mlx90640_RxData(device, reg_addr,  2, data, 3);
    pr_info("Status Register[%x%X] : %x|%x|%x\n",reg_addr[0],reg_addr[1],data[0],data[1],data[2]);
    return 0;
}

uint8_t mlx90640_TxData(struct mlx90640_dev *device, uint8_t *reg, size_t add_len, uint8_t *data, size_t tx_len)
{

    struct i2c_msg msg[1];
    uint8_t *sendData=kmalloc((add_len+tx_len),GFP_KERNEL);
    memcpy((void*)sendData,reg,add_len);
    memcpy((void*)(sendData+add_len),data,tx_len);

    msg[0].addr = device->client->addr;
    msg[0].flags=0;         //  write
    msg[0].len=tx_len;      //  Address is 2 byte coded
    msg[0].buf=reg;

    if(i2c_transfer(device->client->adapter,msg,1)<0)
    {
        pr_err("mlx90640 [%x]: i2c transfer failed\n",device->client->addr);
        kfree(sendData);
        return -ENODEV;
    }
    kfree(sendData);
    pr_info("PID: %x, VER %x\n",data[0],data[1]);
    return 0;

}

uint8_t mlx90640_RxData(struct mlx90640_dev *device, uint8_t *reg,  size_t tx_len, uint8_t *data, size_t rx_len)
{

    struct i2c_msg msg[2];
    uint8_t *buffer;
    buffer = kzalloc(rx_len,GFP_KERNEL);
    if(!buffer)
        return -ENOMEM;
    

    msg[0].addr = device->client->addr;
    msg[0].flags=0;         //  write
    msg[0].len=tx_len;      //  Address is 2 byte coded
    msg[0].buf=reg;

    msg[1].addr = device->client->addr;
    msg[1].flags=I2C_M_RD;
    msg[1].len=rx_len;
    msg[1].buf=data;

    if(i2c_transfer(device->client->adapter,msg,2)<0)
    {
        pr_err("mlx90640 [%x]: i2c transfer failed\n",device->client->addr);
         kfree(buffer);
        return -ENODEV;
    }

    memcpy(data,buffer,rx_len);
    kfree(buffer);
    return 0;
}
