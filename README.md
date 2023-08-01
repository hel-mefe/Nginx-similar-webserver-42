## TO DO ##
- rebuilding is_file function.
- adding 302 redirection if URL does not terminate with slash.
- rebuilding status handlers and not hardcoding them.
- checking KEEP ALIVE in disconnect_client not in get.
- rebuilding fill_response to fit all methods.
- HEAD method still need changes (some headers ares missing ex:"content-length") ...

## IMPORTANT ##
- response is too slow or not stuck, compare with previous versions we should know why.
- response is always served at the end.
