# nginx_http_module_mytest
a simple demo of nginx http module

paste the following conf into a new conf, and include it in the main conf.

server
{
  mytest on; # or off
  mytest_str "XXXXXX"; # your string
  
  ...
}

or in the location block

location XXXXX
{
  mytest on;
  mytest_str "XXXXXX";
  
  ...
}
