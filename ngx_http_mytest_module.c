#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#ifndef offsetof
#define offsetof(type, field) (size_t)(&(((type *)0)->field))
#endif

#ifndef NGX_HTTP_MYTEST_TEXT
#define NGX_HTTP_MYTEST_TEXT "hello,\niam chng trang.\nthis text is sent when mytest_str is not specified\n"
#endif

/*static*/
ngx_str_t default_text = ngx_string(NGX_HTTP_MYTEST_TEXT);

/*********************** defined conf_t ************************/

typedef struct {
	ngx_flag_t my_flag;
	ngx_str_t my_text;
} ngx_http_mytest_conf_t;

/*********************** declare *******************************/

static void * ngx_http_mytest_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void * parent, void *child);

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);

static char * ngx_http_mytest_set_my_flag(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char * ngx_http_mytest_set_my_text(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


/*********************** struct *******************************/

/*static*/ 
ngx_command_t ngx_http_mytest_commands[] = {
	
	{
		/*module name*/	ngx_string("mytest"),
		/*module type*/	NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_FLAG,
		/*setting*/	ngx_http_mytest_set_my_flag,
		/*offset conf*/	NGX_HTTP_LOC_CONF_OFFSET,
		/*offset chap 4*/offsetof(ngx_http_mytest_conf_t, my_flag),
		/*post*/	NULL
	},
	{
		ngx_string("mytest_str"),
		NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		ngx_http_mytest_set_my_text,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_mytest_conf_t, my_text),
		NULL
	},
	/*end of the array*/	ngx_null_command  //empty ngx_command_t
	/*{
		ngx_null_string,
		0,
		NULL,
		0,
		0,
		NULL
	}*/
};

/*static*/
ngx_http_module_t ngx_http_mytest_module_ctx = {
	/*called before resolving the configuration file */
	0,	/* ngx_int_t (*preconfiguration)(ngx_conf_t *cf) */
	/*called after resolving the configuration file */
	0,	/* ngx_int_t (*postconfiguration)(ngx_conf_t *cf) */

	/*create a conf struct for grobal configuration */
	0,	/* void * (*create_main_conf)(ngx_conf_t * cf)*/
	/*initialize the main conf struct */
	0,	/* char * (*init_main_conf)(ngx_conf_t * cf)*/

	0,	/* void * (*create_srv_conf)(ngx_conf_t * cf)*/
	0,	/* char * (*merge_srv_conf)(ngx_conf_t * cf, void * prev, void * conf)*/

	ngx_http_mytest_create_loc_conf,	
		/*create location configuration*/
	ngx_http_mytest_merge_loc_conf,
		/*merge location configuration: enables the same conf apears in different blocks*/
};


ngx_module_t ngx_http_mytest_module = {

	NGX_MODULE_V1, /*0,0,0,0,0,0,1 (version=1 as the default) reserved variables*/
	&ngx_http_mytest_module_ctx,	/* & */
	ngx_http_mytest_commands,	/* array with only one ngx_command_t*/
	NGX_HTTP_MODULE,
	0,	/*init_master: not used*/
	0,	/*init_module*/
	0,	/*init_process*/
	0,	/*init_thread: not support*/
	0,	/*exit_thread: not support*/
	0,	/*exit_process*/
	0,	/*exit_master*/
	NGX_MODULE_V1_PADDING /*0,0,0,0,0,0,0,0, eight reserved variables*/
};



/********************* functions **********************/

/*static*/ 
void * ngx_http_mytest_create_loc_conf(ngx_conf_t *cf)
{

	ngx_http_mytest_conf_t *mycf = (ngx_http_mytest_conf_t*) ngx_pcalloc(cf->pool, sizeof(ngx_http_mytest_conf_t));
	if(!mycf) return NULL;
	//initial values
	mycf->my_flag = NGX_CONF_UNSET;
	mycf->my_text = (ngx_str_t)ngx_null_string; //(ngx_str_t) is required (why?)
	return mycf;
}

/*static*/
char * ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void * parent, void *child)
{
	ngx_http_mytest_conf_t * p = (ngx_http_mytest_conf_t *)parent;
	ngx_http_mytest_conf_t * c = (ngx_http_mytest_conf_t *)child;
	ngx_conf_merge_value(p->my_flag, c->my_flag, 0);
	ngx_conf_merge_str_value((c->my_text), (p->my_text), NGX_HTTP_MYTEST_TEXT);
	return NGX_CONF_OK;
}

/*static*/
ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
	ngx_http_mytest_conf_t * mycf;
	//return 405 if method is not GEt or HEAD
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
		return NGX_HTTP_NOT_ALLOWED;

	//discard the request body
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK)
		return rc;
	
	/**/
	mycf = (ngx_http_mytest_conf_t *)ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);
	if(mycf==0)
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	
	ngx_str_t type = ngx_string("text/plain");
	ngx_str_t response = mycf->my_text;
	r->headers_out.status = NGX_HTTP_OK; //200
	r->headers_out.content_length_n = response.len;
	r->headers_out.content_type = type;
	//for the range protocol
	//r->allow_ranges = 1;

	rc = ngx_http_send_header(r);
	if(rc == NGX_ERROR || rc > NGX_OK || r->header_only)
		return rc;
	//buf_t can be used as buffer or file
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len); //r->pool
	if(b == NULL)
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	
	ngx_memcpy(b->pos, response.data, response.len);
	//set the ptr last
	b->last = b->pos + response.len;
	b->last_buf = 1;


	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);
}


/*static*/
char * ngx_http_mytest_set_my_flag(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	// /*
	ngx_http_core_loc_conf_t *clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	ngx_conf_post_t * post = NULL;
	ngx_str_t * value = NULL;
	ngx_int_t * np = (ngx_int_t *)((char *)conf+(cmd->offset));
	if(clcf == NULL)
		return "failed to get loc_conf";
	if(*np != NGX_CONF_UNSET)
		return "is duplicated";

	value = cf->args->elts; //value -> the configuration parameter value
	if(ngx_strncmp("on", value[1].data, value[1].len)==0)
	{
		clcf->handler = ngx_http_mytest_handler;
		*np = 1;
	}
	else if(ngx_strncmp("off", value[1].data, value[1].len)==0)
	{
		clcf->handler = NULL;
		*np = 0;
	}
	else
	{
		return "invalid parameter";
	}
	post = (ngx_conf_post_t *)cmd->post;
	if(post)
		post->post_handler(cf, cmd->post, np);
	
	return NGX_CONF_OK;
	// */
	//return ngx_conf_set_flag_slot(cf, cmd, conf); //return NULL;
}

/*static*/ 
char * ngx_http_mytest_set_my_text(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	ngx_str_t * value;
	ngx_int_t nvalues;
	ngx_str_t * np = (ngx_str_t *)((char *)conf+(cmd->offset));
	if(np->data)
		return "is duplicated (chng)";
	nvalues = cf->args->nelts;
	value = cf->args->elts; // value -> the configuration value of "my-text";
	if(nvalues < 2)
	{
		//when the parameter is less than 1, use the default text
		/*
		//when using the local variable tmp, pay attention to its lifetime
		np->data = (ngx_str_t*)ngx_pcalloc(cf->pool, tmp.len);
		if(np->data == NULL)
			return "memerror";
		ngx_copy(np->data, tmp.data, tmp.len);
		np->len = tmp.len;
		*/
		*np = default_text;
	}
	else
	{
		*np = value[1];
	}
	
	return NGX_CONF_OK;
}




