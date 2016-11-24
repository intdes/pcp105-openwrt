#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/watchdog.h>
#include <linux/i2c.h>

#include "hyrax_driver.h"
#include "hyrax_i2c.h"

#define HYRAX_WD_TIMEOUT_MIN  1   /* in sec */
#define HYRAX_WD_TIMEOUT_MAX  6553    /* in sec */

static const struct watchdog_info hyrax_wdt_info = {
    .options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
    .identity = DEVICE_NAME,
};

static int hyrax_wdt_start(struct watchdog_device *wdt)
{
	struct hyrax_priv *priv = watchdog_get_drvdata(wdt);
	int ret;

	ret = SetWatchdogTimeout( &priv->i2c->dev, wdt->timeout );
	return ( ret );
}

static int hyrax_wdt_stop(struct watchdog_device *wdt)
{
	struct hyrax_priv *priv = watchdog_get_drvdata(wdt);

	return ( SetWatchdogTimeout( &priv->i2c->dev, 0 ) );
}

static int hyrax_wdt_ping(struct watchdog_device *wdt)
{
	struct hyrax_priv *priv = watchdog_get_drvdata(wdt);

	return ( KickWatchdog( &priv->i2c->dev ) );
}

static int
hyrax_wdt_settimeout(struct watchdog_device *wdt, unsigned int timeout)
{
	struct hyrax_priv *priv = watchdog_get_drvdata(wdt);

    int ret;

	ret = SetWatchdogTimeout( &priv->i2c->dev, timeout );
    if (ret < 0)
        return ret;

    wdt->timeout = timeout;

	return ( ret );
}

static const struct watchdog_ops hyrax_wdt_ops = {
    .owner      = THIS_MODULE,
    .start      = hyrax_wdt_start,
    .stop       = hyrax_wdt_stop,
    .ping       = hyrax_wdt_ping,
    .set_timeout    = hyrax_wdt_settimeout,
};

int hyrax_init_wdt( struct i2c_client *i2c )
{
    struct hyrax_priv *priv = dev_get_drvdata(&i2c->dev);
	int ret;

    priv->wdt.ops = &hyrax_wdt_ops;
    priv->wdt.info = &hyrax_wdt_info;
    priv->wdt.min_timeout = HYRAX_WD_TIMEOUT_MIN;
    priv->wdt.max_timeout = HYRAX_WD_TIMEOUT_MAX;
    priv->wdt.parent = &i2c->dev;
	priv->wdt.timeout = 0;

    watchdog_init_timeout(&priv->wdt, 0, &i2c->dev);
    watchdog_set_nowayout(&priv->wdt, 0);
    watchdog_set_drvdata(&priv->wdt, priv);

    ret = watchdog_register_device(&priv->wdt);
    if (ret) {
        dev_err(&i2c->dev, "failed to register Watchdog device\n");
    }
	return ( ret );
}

void hyrax_close_wdt( struct i2c_client *i2c )
{
    struct hyrax_priv *priv = dev_get_drvdata(&i2c->dev);

	watchdog_unregister_device( &priv->wdt );
}


