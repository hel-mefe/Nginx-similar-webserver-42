404 pages

- always 404 pages are served from the root

let's consider this situation

server: root -> /www
location: /testingLoc - root -> /testingDir

Then 404 will be /testingDir/404.html

But, if I don't have the root at the location then I'll always work with the server root
Which means the path will become /www/404.html