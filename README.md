Hey dont forget upload folder and check in post request if it exist in requested location.
PUT method does not apply for 404 handler so dont forget it in refactoring.
Add child map in manager struct and let it add ponter to the map for every client (add_client function).
in handle_client add condition to check waitpid in return if > 0 && WIFEXITED(stat) is true : add it to child map.
dont forget optimize request/response attributes.
define root and location fullpath in set_response_config function
