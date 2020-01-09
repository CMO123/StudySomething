#include <linux/module.h>
#include <linux/init.h>

#include <linux/delay.h>

#include <linux/slab.h>

typedef struct {
	mempool_t r_rq_pool;

}mempool_test_t;


static struct kmem_cache *test_ws_cache, *test_rec_cache, *test_g_rq_cache,
				*test_w_rq_cache;
static DECLARE_RWSEM(test_lock);

#define test_g_rq_size 64


static int test_init_global_caches()
{
	down_write(&test_lock);
	
	test_g_rq_cache = kmem_cache_create("test_g_rq", test_g_rq_size,
				0, 0, NULL);
	if (!test_g_rq_cache) {
		kmem_cache_destroy(test_ws_cache);
		kmem_cache_destroy(test_rec_cache);
		up_write(&test_lock);
		return -ENOMEM;
	}
	up_write(&test_lock);

	return 0;
}

static void test_free_global_caches()
{
	kmem_cache_destroy(test_g_rq_cache);
}


static int __init mempool_test_init(void)
{
	int ret = 0;

	mempool_test_t p;

	if (test_init_global_caches()) {
		pr_err ("Error init_global_cache\n");
		goto fail;
	}
	
	ret = mempool_init_slab_pool(&p.r_rq_pool, 16,
				     test_g_rq_cache);
	if (ret)
		goto free_global_caches;

	pr_notice("mempool_test_init done!\n");

	// start test
	mempool_exit(&p.r_rq_pool);
	pr_notice("mempool_exit r_rq_pool, round 1, should success\n");
	mempool_exit(&p.r_rq_pool);
	pr_notice("mempool_exit r_rq_pool, round 2\n");

	test_free_global_caches();
	pr_notice("test_free_global_caches, round 1, should success\n");
	test_free_global_caches();
	pr_notice("test_free_global_caches, round 2\n");

	// end test
	return 0;
free_global_caches:
	test_free_global_caches();
fail:
	return -1;
}

static void __exit mempool_test_exit(void)
{
	
}


module_init(mempool_test_init);
module_exit(mempool_test_exit);
MODULE_LICENSE("GPL");

