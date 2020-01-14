#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c/pca954x.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/leds.h>
#include <linux/reboot.h>
#include <linux/delay.h>


#define SEP(XXX) 1
#define IS_INVALID_PTR(_PTR_) ((_PTR_ == NULL) || IS_ERR(_PTR_))
#define IS_VALID_PTR(_PTR_) (!IS_INVALID_PTR(_PTR_))

#if SEP("defines")
#define PCA9548_CHANNEL_NUM     8
#define PCA9548_ADAPT_ID_START  10
#define SFP_NUM                 16
#define PORT_NUM                (24+SFP_NUM)
#endif

#if SEP("i2c:smbus")
static int e550_24t16y_smbus_read_reg(struct i2c_client *client, unsigned char reg, unsigned char* value)
{
    int ret = 0;

    if (IS_INVALID_PTR(client))
    {
        printk(KERN_CRIT "invalid i2c client");
        return -1;
    }
    
    ret = i2c_smbus_read_byte_data(client, reg);
    if (ret >= 0) {
        *value = (unsigned char)ret;
    }
    else
    {
        *value = 0;
        printk(KERN_CRIT "i2c_smbus op failed: ret=%d reg=%d\n",ret ,reg);
        return ret;
    }

    return 0;
}

static int e550_24t16y_smbus_write_reg(struct i2c_client *client, unsigned char reg, unsigned char value)
{
    int ret = 0;
    
    if (IS_INVALID_PTR(client))
    {
        printk(KERN_CRIT "invalid i2c client");
        return -1;
    }
    
    ret = i2c_smbus_write_byte_data(client, reg, value);
    if (ret != 0)
    {
        printk(KERN_CRIT "i2c_smbus op failed: ret=%d reg=%d\n",ret ,reg);
        return ret;
    }

    return 0;
}
#endif

#if SEP("i2c:master")
static struct i2c_adapter *i2c_adp_master          = NULL; /* i2c-0-cpu */

static int e550_24t16y_init_i2c_master(void)
{
    /* find i2c-core master */
    i2c_adp_master = i2c_get_adapter(1);
    if(IS_INVALID_PTR(i2c_adp_master))
    {
        i2c_adp_master = NULL;
        printk(KERN_CRIT "e550_24t16y_init_i2c_master can't find i2c-core bus\n");
        return -1;
    }
    
    return 0;
}

static int e550_24t16y_exit_i2c_master(void)
{
    /* uninstall i2c-core master */
    if(IS_VALID_PTR(i2c_adp_master)) {
        i2c_put_adapter(i2c_adp_master);
        i2c_adp_master = NULL;
    }
    
    return 0;
}
#endif

#if SEP("i2c:pca9548")
static struct pca954x_platform_mode i2c_dev_pca9548_platform_mode[PCA9548_CHANNEL_NUM] = {
    [0] = {
        .adap_id = PCA9548_ADAPT_ID_START,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [1] = {
        .adap_id = PCA9548_ADAPT_ID_START + 1,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [2] = {
        .adap_id = PCA9548_ADAPT_ID_START + 2,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [3] = {
        .adap_id = PCA9548_ADAPT_ID_START + 3,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [4] = {
        .adap_id = PCA9548_ADAPT_ID_START + 4,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [5] = {
        .adap_id = PCA9548_ADAPT_ID_START + 5,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [6] = {
        .adap_id = PCA9548_ADAPT_ID_START + 6,
        .deselect_on_exit = 1,
        .class   = 0,
    },
    [7] = {
        .adap_id = PCA9548_ADAPT_ID_START + 7,
        .deselect_on_exit = 1,
        .class   = 0,
    }
};
static struct pca954x_platform_data i2c_dev_pca9548_platform_data = {
    .modes = i2c_dev_pca9548_platform_mode,
    .num_modes = PCA9548_CHANNEL_NUM,
};
static struct i2c_board_info i2c_dev_pca9548 = {
    I2C_BOARD_INFO("pca9548", 0x70),
    .platform_data = &i2c_dev_pca9548_platform_data,
};
static struct i2c_client  *i2c_client_pca9548x     = NULL;

static int e550_24t16y_init_i2c_pca9548(void)
{
    if(IS_INVALID_PTR(i2c_adp_master))
    {
        i2c_adp_master = NULL;
        printk(KERN_CRIT "e550_24t16y_init_i2c_pca9548 can't find i2c-core bus\n");
        return -1;
    }

    /* install i2c-mux */
    i2c_client_pca9548x = i2c_new_device(i2c_adp_master, &i2c_dev_pca9548);
    if(IS_INVALID_PTR(i2c_client_pca9548x))
    {
        i2c_client_pca9548x = NULL;
        printk(KERN_CRIT "install e550_24t16y board pca9548 failed\n");
        return -1;
    }
    
    return 0;
}

static int e550_24t16y_exit_i2c_pca9548(void)
{
    /* uninstall i2c-core master */
    if(IS_VALID_PTR(i2c_client_pca9548x)) {
        i2c_unregister_device(i2c_client_pca9548x);
        i2c_client_pca9548x = NULL;
    }
    
    return 0;
}
#endif

#if SEP("i2c:adt7470")
static struct i2c_board_info i2c_dev_adt7470 = {
    I2C_BOARD_INFO("adt7470", 0x2F),
};
static struct i2c_adapter *i2c_adp_adt7470         = NULL; /* pca9548x-channel 5 */
static struct i2c_client  *i2c_client_adt7470      = NULL;

static int e550_24t16y_init_i2c_adt7470(void)
{
    i2c_adp_adt7470 = i2c_get_adapter(PCA9548_ADAPT_ID_START + 5);
    if(IS_INVALID_PTR(i2c_adp_adt7470))
    {
        i2c_adp_adt7470 = NULL;
        printk(KERN_CRIT "install e550_24t16y board adt7470 failed\n");
        return -1;
    }
    
    i2c_client_adt7470 = i2c_new_device(i2c_adp_adt7470, &i2c_dev_adt7470);
    if(IS_INVALID_PTR(i2c_client_adt7470)){
        i2c_client_adt7470 = NULL;
        printk(KERN_CRIT "install e550_24t16y board adt7470 failed\n");
        return -1;
    }
    
    return 0;
}

static int e550_24t16y_exit_i2c_adt7470(void)
{
    if(IS_VALID_PTR(i2c_client_adt7470)) {
        i2c_unregister_device(i2c_client_adt7470);
        i2c_client_adt7470 = NULL;
    }
    
    if(IS_VALID_PTR(i2c_adp_adt7470)) {
        i2c_put_adapter(i2c_adp_adt7470);
        i2c_adp_adt7470 = NULL;
    }
    
    return 0;
}
#endif

#if SEP("i2c:lm77")
static struct i2c_board_info i2c_dev_sensor1 = {
    I2C_BOARD_INFO("lm77", 0x49),
};
static struct i2c_board_info i2c_dev_sensor2 = {
    I2C_BOARD_INFO("lm77", 0x48),
};

static struct i2c_adapter *i2c_adp_sensor1         = NULL;
static struct i2c_adapter *i2c_adp_sensor2         = NULL;
static struct i2c_client  *i2c_client_sensor1      = NULL;
static struct i2c_client  *i2c_client_sensor2      = NULL;

static int e550_24t16y_init_i2c_sensor(void)
{
    i2c_adp_sensor1 = i2c_get_adapter(PCA9548_ADAPT_ID_START + 6);
    if(IS_INVALID_PTR(i2c_adp_sensor1))
    {
        i2c_adp_sensor1 = NULL;
        printk(KERN_CRIT "install e550_24t16y board sensor1 failed\n");
        return -1;
    }

    i2c_adp_sensor2 = i2c_get_adapter(PCA9548_ADAPT_ID_START + 7);
    if(IS_INVALID_PTR(i2c_adp_sensor2))
    {
        i2c_adp_sensor2 = NULL;
        printk(KERN_CRIT "install e550_24t16y board sensor2 failed\n");
        return -1;
    }

    i2c_client_sensor1 = i2c_new_device(i2c_adp_sensor1, &i2c_dev_sensor1);
    if(IS_INVALID_PTR(i2c_client_sensor1)){
        i2c_client_sensor1 = NULL;
        printk(KERN_CRIT "install e550_24t16y board sensor1 failed\n");
        return -1;
    }

    i2c_client_sensor2 = i2c_new_device(i2c_adp_sensor2, &i2c_dev_sensor2);
    if(IS_INVALID_PTR(i2c_client_sensor2)){
        i2c_client_sensor2 = NULL;
        printk(KERN_CRIT "install e550_24t16y board sensor2 failed\n");
        return -1;
    }

    return 0;
}

static int e550_24t16y_exit_i2c_sensor(void)
{
    if(IS_VALID_PTR(i2c_client_sensor1)) {
        i2c_unregister_device(i2c_client_sensor1);
        i2c_client_sensor1 = NULL;
    }

    if(IS_VALID_PTR(i2c_adp_sensor1)) {
        i2c_put_adapter(i2c_adp_sensor1);
        i2c_adp_sensor1 = NULL;
    }

    if(IS_VALID_PTR(i2c_client_sensor2)) {
        i2c_unregister_device(i2c_client_sensor2);
        i2c_client_sensor2 = NULL;
    }

    if(IS_VALID_PTR(i2c_adp_sensor2)) {
        i2c_put_adapter(i2c_adp_sensor2);
        i2c_adp_sensor2 = NULL;
    }

    return 0;
}
#endif

#if SEP("i2c:psu")
static struct i2c_adapter *i2c_adp_psu1           = NULL; /* psu1 channel 3 */
static struct i2c_adapter *i2c_adp_psu2           = NULL; /* psu2 channel 1 */
static struct i2c_board_info i2c_dev_psu1 = {
    I2C_BOARD_INFO("i2c-psu1", 0x50), //0xa0
};
static struct i2c_board_info i2c_dev_psu2 = {
    I2C_BOARD_INFO("i2c-psu2", 0x51), //0xa2
};
static struct i2c_client  *i2c_client_psu1        = NULL;
static struct i2c_client  *i2c_client_psu2        = NULL;

static int e550_24t16y_init_i2c_psu(void)
{
    i2c_adp_psu1 = i2c_get_adapter(PCA9548_ADAPT_ID_START + 3);
    if(IS_INVALID_PTR(i2c_adp_psu1))
    {
        i2c_adp_psu1 = NULL;
        printk(KERN_CRIT "get e550_24t16y psu1 i2c-adp failed\n");
        return -1;
    }

    i2c_adp_psu2 = i2c_get_adapter(PCA9548_ADAPT_ID_START + 2);
    if(IS_INVALID_PTR(i2c_adp_psu2))
    {
        i2c_adp_psu2 = NULL;
        printk(KERN_CRIT "get e550_24t16y psu2 i2c-adp failed\n");
        return -1;
    }
    
    i2c_client_psu1 = i2c_new_device(i2c_adp_psu1, &i2c_dev_psu1);
    if(IS_INVALID_PTR(i2c_client_psu1)){
        i2c_client_psu1 = NULL;
        printk(KERN_CRIT "create e550_24t16y board i2c client psu1 failed\n");
        return -1;
    }

    i2c_client_psu2 = i2c_new_device(i2c_adp_psu2, &i2c_dev_psu2);
    if(IS_INVALID_PTR(i2c_client_psu2)){
        i2c_client_psu2 = NULL;
        printk(KERN_CRIT "create e550_24t16y board i2c client psu2 failed\n");
        return -1;
    }
    
    return 0;
}

static int e550_24t16y_exit_i2c_psu(void)
{
    if(IS_VALID_PTR(i2c_client_psu1)) {
        i2c_unregister_device(i2c_client_psu1);
        i2c_client_psu1 = NULL;
    }

    if(IS_VALID_PTR(i2c_client_psu2)) {
        i2c_unregister_device(i2c_client_psu2);
        i2c_client_psu2 = NULL;
    }

    if(IS_VALID_PTR(i2c_adp_psu1)) 
    {
        i2c_put_adapter(i2c_adp_psu1);
        i2c_adp_psu1 = NULL;
    }

    if(IS_VALID_PTR(i2c_adp_psu2)) 
    {
        i2c_put_adapter(i2c_adp_psu2);
        i2c_adp_psu2 = NULL;
    }
    
    return 0;
}
#endif

#if SEP("reboot")
extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);
void (*arm_pm_restart_old)(enum reboot_mode reboot_mode, const char *cmd) = NULL;
#endif


#if SEP("i2c:epld")
static struct i2c_board_info i2c_dev_epld = {
    I2C_BOARD_INFO("i2c-epld", 0x36),
};
static struct i2c_client  *i2c_client_epld      = NULL;

void epld_reboot(enum reboot_mode mode, const char *cmd)
{
    if(IS_INVALID_PTR(i2c_client_epld))
    {
        i2c_client_epld = NULL;
        printk(KERN_CRIT "cannot reboot by epld\n");
        while (1)
            ; /* loop */
    }

    e550_24t16y_smbus_write_reg(i2c_client_epld, 0x23, 0x0);
    mdelay(200);
    e550_24t16y_smbus_write_reg(i2c_client_epld, 0x23, 0x3);
    while (1)
        ; /* loop */

    return;
}


static int e550_24t16y_init_i2c_epld(void)
{
    unsigned char value = 0;
    int ret = 0;
    if (IS_INVALID_PTR(i2c_adp_master))
    {
         printk(KERN_CRIT "e550_24t16y_init_i2c_epld can't find i2c-core bus\n");
         return -1;
    }
    
    i2c_client_epld = i2c_new_device(i2c_adp_master, &i2c_dev_epld);
    if(IS_INVALID_PTR(i2c_client_epld))
    {
        i2c_client_epld = NULL;
        printk(KERN_CRIT "create e550_24t16y board i2c client epld failed\n");
        return -1;
    }

    /* 1. release i2c bridge */
    ret = e550_24t16y_smbus_read_reg(i2c_client_epld, 0x8, &value);
    value |= (0x20);
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0x8, value);
    /* 2. release phy */
    ret += e550_24t16y_smbus_read_reg(i2c_client_epld, 0x7, &value);
    value |= (0x3);
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0x7, value);
    /* 3. release phy intr mask, ppu intr mask */
    ret += e550_24t16y_smbus_read_reg(i2c_client_epld, 0xb, &value);
    value &= (0xfc);
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0xb, value);
    ret += e550_24t16y_smbus_read_reg(i2c_client_epld, 0xf, &value);
    value &= (0xf0);
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0xf, value);
    /* 4. set sfp tx enable */
    value = 0x00;
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0x13, value);
    value = 0x00;
    ret += e550_24t16y_smbus_write_reg(i2c_client_epld, 0x14, value);

    if (ret != 0)
    {
        printk(KERN_CRIT "init e550_24t16y board epld config failed\n");
        return -1;
    }


    /* init reboot hook for ls1023a */
    arm_pm_restart_old = arm_pm_restart;
    arm_pm_restart = epld_reboot;

    return 0;
}

static int e550_24t16y_exit_i2c_epld(void)
{
    if(IS_VALID_PTR(i2c_client_epld)) {
        arm_pm_restart = arm_pm_restart_old;
        i2c_unregister_device(i2c_client_epld);
        i2c_client_epld = NULL;
    }
    
    return 0;
}
#endif


#if SEP("drivers:psu")
static struct class* psu_class = NULL;
static struct device* psu_dev_psu1 = NULL;
static struct device* psu_dev_psu2 = NULL;

static ssize_t e550_24t16y_psu_read_presence(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    unsigned char offset = 0;
    unsigned char present_no = 0;
    unsigned char present = 0;
    unsigned char value = 0;
    struct i2c_client *i2c_psu_client = NULL;

    if (psu_dev_psu1 == dev)
    {
        i2c_psu_client = i2c_client_epld;
        offset = 0x1e;
        present_no = 2;
    }
    else if (psu_dev_psu2 == dev)
    {
        i2c_psu_client = i2c_client_epld;
        offset = 0x1e;
        present_no = 3;
    }
    else
    {
        return sprintf(buf, "Error: unknown psu device\n");
    }

    if (IS_INVALID_PTR(i2c_psu_client))
    {
        return sprintf(buf, "Error: psu i2c-adapter invalid\n");
    }

    ret = e550_24t16y_smbus_read_reg(i2c_psu_client, offset, &present);
    if (ret != 0)
    {
        return sprintf(buf, "Error: read psu data:%s failed\n", attr->attr.name);
    }

    value = ((present & (1<<(present_no%8))) ? 1 : 0 ); //0: absent 1: present
    
    return sprintf(buf, "%d\n", value);
}

static ssize_t e550_24t16y_psu_read_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    unsigned char offset = 0;
    unsigned char workstate_no = 0;
    unsigned char workstate = 0;
    unsigned char value = 0;
    struct i2c_client *i2c_psu_client = NULL;

    if (psu_dev_psu1 == dev)
    {
        i2c_psu_client = i2c_client_epld;
        offset = 0x1e;
        workstate_no = 4;
    }
    else if (psu_dev_psu2 == dev)
    {
        i2c_psu_client = i2c_client_epld;
        offset = 0x1e;
        workstate_no = 5;
    }
    else
    {
        return sprintf(buf, "Error: unknown psu device\n");
    }

    if (IS_INVALID_PTR(i2c_psu_client))
    {
        return sprintf(buf, "Error: psu i2c-adapter invalid\n");
    }

    ret = e550_24t16y_smbus_read_reg(i2c_psu_client, offset, &workstate);
    if (ret != 0)
    {
        return sprintf(buf, "Error: read psu data:%s failed\n", attr->attr.name);
    }

    value = ((workstate & (1<<(workstate_no%8))) ? 0 : 1 ); //0: fail 1: fine
    
    return sprintf(buf, "%d\n", value);
}

static DEVICE_ATTR(psu_presence, S_IRUGO, e550_24t16y_psu_read_presence, NULL);
static DEVICE_ATTR(psu_status, S_IRUGO, e550_24t16y_psu_read_status, NULL);

static int e550_24t16y_init_psu(void)
{
    int ret = 0;
    
    psu_class = class_create(THIS_MODULE, "psu");
    if (IS_INVALID_PTR(psu_class))
    {
        psu_class = NULL;
        printk(KERN_CRIT "create e550_24t16y class psu failed\n");
        return -1;
    }

    psu_dev_psu1 = device_create(psu_class, NULL, MKDEV(222,0), NULL, "psu1");
    if (IS_INVALID_PTR(psu_dev_psu1))
    {
        psu_dev_psu1 = NULL;
        printk(KERN_CRIT "create e550_24t16y psu1 device failed\n");
        return -1;
    }

    psu_dev_psu2 = device_create(psu_class, NULL, MKDEV(222,1), NULL, "psu2");
    if (IS_INVALID_PTR(psu_dev_psu2))
    {
        psu_dev_psu2 = NULL;
        printk(KERN_CRIT "create e550_24t16y psu2 device failed\n");
        return -1;
    }

    ret = device_create_file(psu_dev_psu1, &dev_attr_psu_presence);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y psu1 device attr:presence failed\n");
        return -1;
    }

    ret = device_create_file(psu_dev_psu1, &dev_attr_psu_status);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y psu1 device attr:status failed\n");
        return -1;
    }

    ret = device_create_file(psu_dev_psu2, &dev_attr_psu_presence);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y psu2 device attr:presence failed\n");
        return -1;
    }

    ret = device_create_file(psu_dev_psu2, &dev_attr_psu_status);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y psu2 device attr:status failed\n");
        return -1;
    }
    
    return 0;
}

static int e550_24t16y_exit_psu(void)
{
    if (IS_VALID_PTR(psu_dev_psu1))
    {
        device_remove_file(psu_dev_psu1, &dev_attr_psu_presence);
        device_remove_file(psu_dev_psu1, &dev_attr_psu_status);
        device_destroy(psu_class, MKDEV(222,0));
    }

    if (IS_VALID_PTR(psu_dev_psu2))
    {
        device_remove_file(psu_dev_psu2, &dev_attr_psu_presence);
        device_remove_file(psu_dev_psu2, &dev_attr_psu_status);
        device_destroy(psu_class, MKDEV(222,1));
    }

    if (IS_VALID_PTR(psu_class))
    {
        class_destroy(psu_class);
        psu_class = NULL;
    }

    return 0;
}
#endif


#if SEP("drivers:leds")
extern void e550_24t16y_led_set(struct led_classdev *led_cdev, enum led_brightness set_value);
extern enum led_brightness e550_24t16y_led_get(struct led_classdev *led_cdev);
extern void e550_24t16y_led_port_set(struct led_classdev *led_cdev, enum led_brightness set_value);
extern enum led_brightness e550_24t16y_led_port_get(struct led_classdev *led_cdev);

static struct led_classdev led_dev_system = {
    .name = "system",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_idn = {
    .name = "idn",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_fan1 = {
    .name = "fan1",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_fan2 = {
    .name = "fan2",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_fan3 = {
    .name = "fan3",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_fan4 = {
    .name = "fan4",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_psu1 = {
    .name = "psu1",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_psu2 = {
    .name = "psu2",
    .brightness_set = e550_24t16y_led_set,
    .brightness_get = e550_24t16y_led_get,
};
static struct led_classdev led_dev_port[PORT_NUM] = {
{   .name = "port1",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port2",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port3",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port4",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port5",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port6",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port7",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port8",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port9",     .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port10",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port11",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port12",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port13",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port14",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port15",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port16",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port17",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port18",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port19",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port20",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port21",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port22",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port23",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port24",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port25",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port26",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port27",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port28",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port29",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port30",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port31",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port32",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port33",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port34",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port35",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port36",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port37",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port38",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port39",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
{   .name = "port40",    .brightness_set = e550_24t16y_led_port_set,    .brightness_get = e550_24t16y_led_port_get,},
};
static unsigned char port_led_mode[PORT_NUM] = {0};

void e550_24t16y_led_set(struct led_classdev *led_cdev, enum led_brightness set_value)
{
    int ret = 0;
    unsigned char reg = 0;
    unsigned char mask = 0;
    unsigned char shift = 0;
    unsigned char led_value = 0;
    struct i2c_client *i2c_led_client = i2c_client_epld;

    if (0 == strcmp(led_dev_system.name, led_cdev->name))
    {
        reg = 0x2;
        mask = 0x0F;
        shift = 4;
    }
    else if (0 == strcmp(led_dev_idn.name, led_cdev->name))
    {
        reg = 0x3;
        mask = 0xFE;
        shift = 0;
    }
    else if (0 == strcmp(led_dev_fan1.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan2.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan3.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan4.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_psu1.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_psu2.name, led_cdev->name))
    {
        goto not_support;
    }
    else
    {
        goto not_support;
    }

    ret = e550_24t16y_smbus_read_reg(i2c_led_client, reg, &led_value);
    if (ret != 0)
    {
        printk(KERN_CRIT "Error: read %s led attr failed\n", led_cdev->name);
        return;
    }

    led_value = ((led_value & mask) | ((set_value << shift) & (~mask)));
    
    ret = e550_24t16y_smbus_write_reg(i2c_led_client, reg, led_value);
    if (ret != 0)
    {
        printk(KERN_CRIT "Error: write %s led attr failed\n", led_cdev->name);
        return;
    }

    return;
    
not_support:

    printk(KERN_INFO "Error: not support device:%s\n", led_cdev->name);
    return;
}

enum led_brightness e550_24t16y_led_get(struct led_classdev *led_cdev)
{
    int ret = 0;
    unsigned char reg = 0;
    unsigned char mask = 0;
    unsigned char shift = 0;
    unsigned char led_value = 0;
    struct i2c_client *i2c_led_client = i2c_client_epld;

    if (0 == strcmp(led_dev_system.name, led_cdev->name))
    {
        reg = 0x2;
        mask = 0xF0;
        shift = 4;
    }
    else if (0 == strcmp(led_dev_idn.name, led_cdev->name))
    {
        reg = 0x3;
        mask = 0x01;
        shift = 0;
    }
    else if (0 == strcmp(led_dev_fan1.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan2.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan3.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_fan4.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_psu1.name, led_cdev->name))
    {
        goto not_support;
    }
    else if (0 == strcmp(led_dev_psu2.name, led_cdev->name))
    {
        goto not_support;
    }
    else
    {
        goto not_support;
    }

    ret = e550_24t16y_smbus_read_reg(i2c_led_client, reg, &led_value);
    if (ret != 0)
    {
        printk(KERN_CRIT "Error: read %s led attr failed\n", led_cdev->name);
        return 0;
    }

    led_value = ((led_value & mask) >> shift);

    return led_value;
    
not_support:

    printk(KERN_INFO "Error: not support device:%s\n", led_cdev->name);
    return 0;
}

void e550_24t16y_led_port_set(struct led_classdev *led_cdev, enum led_brightness set_value)
{
    int portNum = 0;
    
    sscanf(led_cdev->name, "port%d", &portNum);
    
    port_led_mode[portNum-1] = set_value;

    return;
}

enum led_brightness e550_24t16y_led_port_get(struct led_classdev *led_cdev)
{
    int portNum = 0;
    
    sscanf(led_cdev->name, "port%d", &portNum);    
    
    return port_led_mode[portNum-1];
}

static int e550_24t16y_init_led(void)
{
    int ret = 0;
    int i = 0;

    ret = led_classdev_register(NULL, &led_dev_system);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_system device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_idn);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_idn device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_fan1);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_fan1 device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_fan2);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_fan2 device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_fan3);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_fan3 device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_fan4);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_fan4 device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_psu1);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_psu1 device failed\n");
        return -1;
    }

    ret = led_classdev_register(NULL, &led_dev_psu2);
    if (ret != 0)
    {
        printk(KERN_CRIT "create e550_24t16y led_dev_psu2 device failed\n");
        return -1;
    }

    for (i=0; i<PORT_NUM; i++)
    {
        ret = led_classdev_register(NULL, &(led_dev_port[i]));
        if (ret != 0)
        {
            printk(KERN_CRIT "create e550_24t16y led_dev_port%d device failed\n", i);
            continue;
        }
    }
    
    return ret;
}

static int e550_24t16y_exit_led(void)
{
    int i = 0;
	
    led_classdev_unregister(&led_dev_system);
    led_classdev_unregister(&led_dev_idn);
    led_classdev_unregister(&led_dev_fan1);
    led_classdev_unregister(&led_dev_fan2);
    led_classdev_unregister(&led_dev_fan3);
    led_classdev_unregister(&led_dev_fan4);
    led_classdev_unregister(&led_dev_psu1);
    led_classdev_unregister(&led_dev_psu2);

    for (i=0; i<PORT_NUM; i++)
    {
        led_classdev_unregister(&(led_dev_port[i]));
    }

    return 0;
}
#endif

#if SEP("drivers:sfp")
#define MAX_SFP_EEPROM_DATA_LEN 256
struct sfp_info_t {
    char data[MAX_SFP_EEPROM_DATA_LEN+1];
    unsigned short data_len;
    int presence;
    spinlock_t lock;
};
static struct class* sfp_class = NULL;
static struct device* sfp_dev[SFP_NUM+1] = {NULL};
static struct sfp_info_t sfp_info[SFP_NUM+1];

static ssize_t e550_24t16y_sfp_read_presence(struct device *dev, struct device_attribute *attr, char *buf)
{
    int portNum = 0;
    const char *name = dev_name(dev);
    unsigned long flags = 0;
    int presence = 0;

    sscanf(name, "sfp%d", &portNum);

    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp read presence, invalid port number!\n");
        buf[0] = '\0';
        return 0;
    }

    spin_lock_irqsave(&(sfp_info[portNum].lock), flags);
    presence = sfp_info[portNum].presence;
    spin_unlock_irqrestore(&(sfp_info[portNum].lock), flags);
    return sprintf(buf, "%d\n", presence);
}

static ssize_t e550_24t16y_sfp_write_presence(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int portNum = 0;
    const char *name = dev_name(dev);
    unsigned long flags = 0;
    int presence = simple_strtol(buf, NULL, 10);

    sscanf(name, "sfp%d", &portNum);

    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp read presence, invalid port number!\n");
        return size;
    }

    spin_lock_irqsave(&(sfp_info[portNum].lock), flags);
    sfp_info[portNum].presence = presence;
    spin_unlock_irqrestore(&(sfp_info[portNum].lock), flags);

    return size;
}

static ssize_t e550_24t16y_sfp_read_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    unsigned char value = 0;
    unsigned char offset = 0;
    unsigned char reg_no = 0;
    int portNum = 0;
    const char *name = dev_name(dev);
    struct i2c_client *i2c_sfp_client = NULL;

    sscanf(name, "sfp%d", &portNum);


    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp read enable, invalid port number!\n");
        value = 0;
    }

    if ((portNum >= 1) && (portNum <= 8))
    {
        offset = 0x14;
        reg_no = portNum - 1;
        i2c_sfp_client = i2c_client_epld;
    }
    else if ((portNum >= 9) && (portNum <= 16))
    {
        offset = 0x13;
        reg_no = portNum - 9;
        i2c_sfp_client = i2c_client_epld;
    }
    else 
    {
        printk(KERN_INFO "%s not supported!\n", name);
        return sprintf(buf, "%d\n", 0);
    }

    ret = e550_24t16y_smbus_read_reg(i2c_sfp_client, offset, &value);
    if (ret != 0)
    {
        return sprintf(buf, "Error: read sfp data:%s failed\n", attr->attr.name);
    }

    value = ((value & (1<<(reg_no%8))) ? 0 : 1 );
    
    return sprintf(buf, "%d\n", value);
}

static ssize_t e550_24t16y_sfp_write_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int ret = 0;
    unsigned char offset = 0;
    unsigned char value = 0;
    unsigned char set_value = simple_strtol(buf, NULL, 10);
    unsigned char reg_no = 0;
    int portNum = 0;
    const char *name = dev_name(dev);
    struct i2c_client *i2c_sfp_client = NULL;

    sscanf(name, "sfp%d", &portNum);

    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp read enable, invalid port number!\n");
        return size;
    }

    if ((portNum >= 1) && (portNum <= 8))
    {
        offset = 0x14;
        reg_no = portNum - 1;
        i2c_sfp_client = i2c_client_epld;
    }
    else if ((portNum >= 9) && (portNum <= 16))
    {
        offset = 0x13;
        reg_no = portNum - 9;
        i2c_sfp_client = i2c_client_epld;
    }
    else 
    {
        printk(KERN_INFO "%s not supported!\n", name);
        return size;
    }

    set_value = ((set_value > 0) ? 0 : 1);

    ret = e550_24t16y_smbus_read_reg(i2c_sfp_client, offset, &value);
    if (ret != 0)
    {
        printk(KERN_CRIT "Error: read %s enable failed\n", name);
        return size;
    }

    if (set_value)
    {
        value = (value | (1<<(reg_no % 8)));
    }
    else
    {
        value = (value & (~(1<<(reg_no % 8))));
    }

    ret = e550_24t16y_smbus_write_reg(i2c_sfp_client, offset, value);
    if (ret != 0)
    {
        printk(KERN_CRIT "Error: read %s write failed\n", name);
        return size;
    }
    
    return size;
}

static ssize_t e550_24t16y_sfp_read_eeprom(struct device *dev, struct device_attribute *attr, char *buf)
{
    int portNum = 0;
    const char *name = dev_name(dev);
    unsigned long flags = 0;
    size_t size = 0;

    sscanf(name, "sfp%d", &portNum);

    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp read eeprom, invalid port number!\n");
        buf[0] = '\0';
        return 0;
    }

    spin_lock_irqsave(&(sfp_info[portNum].lock), flags);
    memcpy(buf, sfp_info[portNum].data, sfp_info[portNum].data_len);
    size = sfp_info[portNum].data_len;
    spin_unlock_irqrestore(&(sfp_info[portNum].lock), flags);

    return size;
}

static ssize_t e550_24t16y_sfp_write_eeprom(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int portNum = 0;
    const char *name = dev_name(dev);
    unsigned long flags = 0;

    sscanf(name, "sfp%d", &portNum);

    if ((portNum < 1) || (portNum > SFP_NUM))
    {
        printk(KERN_CRIT "sfp write eeprom, invalid port number!\n");
        return size;
    }

    spin_lock_irqsave(&(sfp_info[portNum].lock), flags);
    memcpy(sfp_info[portNum].data, buf, size);
    sfp_info[portNum].data_len = size;
    spin_unlock_irqrestore(&(sfp_info[portNum].lock), flags);

    return size;
}

static DEVICE_ATTR(sfp_presence, S_IRUGO|S_IWUSR, e550_24t16y_sfp_read_presence, e550_24t16y_sfp_write_presence);
static DEVICE_ATTR(sfp_enable, S_IRUGO|S_IWUSR, e550_24t16y_sfp_read_enable, e550_24t16y_sfp_write_enable);
static DEVICE_ATTR(sfp_eeprom, S_IRUGO|S_IWUSR, e550_24t16y_sfp_read_eeprom, e550_24t16y_sfp_write_eeprom);

static int e550_24t16y_init_sfp(void)
{
    int ret = 0;
    int i = 0;
    
    sfp_class = class_create(THIS_MODULE, "sfp");
    if (IS_INVALID_PTR(sfp_class))
    {
        sfp_class = NULL;
        printk(KERN_CRIT "create e550_24t16y class sfp failed\n");
        return -1;
    }

    for (i=1; i<=SFP_NUM; i++)
    {
	memset(&(sfp_info[i].data), 0, MAX_SFP_EEPROM_DATA_LEN+1);
        sfp_info[i].data_len = 0;
        spin_lock_init(&(sfp_info[i].lock));

        sfp_dev[i] = device_create(sfp_class, NULL, MKDEV(223,i), NULL, "sfp%d", i);
        if (IS_INVALID_PTR(sfp_dev[i]))
        {
            sfp_dev[i] = NULL;
            printk(KERN_CRIT "create e550_24t16y sfp[%d] device failed\n", i);
            continue;
        }

        ret = device_create_file(sfp_dev[i], &dev_attr_sfp_presence);
        if (ret != 0)
        {
            printk(KERN_CRIT "create e550_24t16y sfp[%d] device attr:presence failed\n", i);
            continue;
        }

        ret = device_create_file(sfp_dev[i], &dev_attr_sfp_enable);
        if (ret != 0)
        {
            printk(KERN_CRIT "create e550_24t16y sfp[%d] device attr:enable failed\n", i);
            continue;
        }

        ret = device_create_file(sfp_dev[i], &dev_attr_sfp_eeprom);
        if (ret != 0)
        {
            printk(KERN_CRIT "create e550_24t16y sfp[%d] device attr:eeprom failed\n", i);
            continue;
        }
    }
    
    return ret;
}

static int e550_24t16y_exit_sfp(void)
{
    int i = 0;

    for (i=1; i<=SFP_NUM; i++)
    {
        if (IS_VALID_PTR(sfp_dev[i]))
        {
            device_remove_file(sfp_dev[i], &dev_attr_sfp_presence);
            device_remove_file(sfp_dev[i], &dev_attr_sfp_enable);
            device_destroy(sfp_class, MKDEV(223,i));
            sfp_dev[i] = NULL;
        }
    }

    if (IS_VALID_PTR(sfp_class))
    {
        class_destroy(sfp_class);
        sfp_class = NULL;
    }

    return 0;
}
#endif

static int e550_24t16y_init(void)
{
    int ret = 0;
    int failed = 0;
    
    printk(KERN_ALERT "install e550_24t16y board dirver...\n");
    
    ret = e550_24t16y_init_i2c_master();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_i2c_epld();
    if (ret != 0)
    {
        failed = 1;
    }
    
    ret = e550_24t16y_init_i2c_pca9548();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_i2c_sensor();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_i2c_adt7470();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_i2c_psu();
    if (ret != 0)
    {
        failed = 1;
    }


    ret = e550_24t16y_init_psu();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_led();
    if (ret != 0)
    {
        failed = 1;
    }

    ret = e550_24t16y_init_sfp();
    if (ret != 0)
    {
        failed = 1;
    }

    if (failed)
        printk(KERN_INFO "install e550_24t16y board driver failed\n");
    else
        printk(KERN_ALERT "install e550_24t16y board dirver...ok\n");
    
    return 0;
}

static void e550_24t16y_exit(void)
{
    printk(KERN_INFO "uninstall e550_24t16y board dirver...\n");
    
    e550_24t16y_exit_sfp();
    e550_24t16y_exit_led();
    e550_24t16y_exit_psu();
    e550_24t16y_exit_i2c_psu();
    e550_24t16y_exit_i2c_adt7470();
    e550_24t16y_exit_i2c_sensor();
    e550_24t16y_exit_i2c_pca9548();
    e550_24t16y_exit_i2c_epld();
    e550_24t16y_exit_i2c_master();
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("shil centecNetworks, Inc");
MODULE_DESCRIPTION("e550-24t16y board driver");
module_init(e550_24t16y_init);
module_exit(e550_24t16y_exit);

