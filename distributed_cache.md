Distributed cache algorithm
==========

# Informal description #

On each node, there would be a table of references, based on a unique hash of
the page (possibly the page address), which would provide either a local
reference or a remote reference; in the case of a local reference, this would be
the actual page requested.  In the case of a remote reference, it would have the
ID, possibly the local IP address and other information for requesting the page
from the other node.  As an alternative option, the node could forward the
request to another node to serve.  This has the downside of potentially
overloading a given node, which sugggests node-pooling would be an option for
better cache responsiveness; possibly caching commonly-accessed resources on
every node, depending on the frequency of calls to it.

As for the replacement mechanism, each cached page would store a removal time,
and a thread would kill the oldest pages from the cache, notifying all nodes via
multicast that the page has been killed.  Likewise, pages can be stored in a
list of last-accessed; whenever a page is read, its TTL is updated and it is
moved to the head of the list.  If the cache is full and a page must be read in
(it does not exist in the hashtable), pages will be killed from the cache in
order of least-recently-accessed-first; each page will be removed and a
notification sent out to all nodes to remove that page from their hashtables;
alternately, nodes could return a "not found" upon other nodes seeking the
page from it, though that could get messy with node pools.  Pages will be
removed from a full cache until there is at least enough space to insert the new
page, but no more than absolutely necessary (if there is enough space to add the
page, do not remove pages.)

A page is loaded into the cache if the hashtable returns a null value for a
page, i.e. it is not loaded on any node.  When a page is loaded into a cache, it
multicasts that information to all other nodes, who all add it to their
hashtables.

I consider this approach to be sensible as it will only load pages when they are
requested, and only hold them when they are requested often.  It is also
possible that the TTL could be adjusted based on load; when load is high,
shorter TTL is preferred to keep the most active pages loaded, and otherwise
swap out less-frequently accessed pages.  Additionally, the garbage-collecting
thread could be adjusted to run more or less regularly, depending on how rapid
cache turnover is or how heavily loaded the cache is.  This way, pages are only
loaded into one cache at a time, while all caches are notified of where to find
the file; that being said, it's not clear that serving from a secondary cache
would be faster than serving from the main server in that case.  For that reason
I'm considering the possibility of multiple nodes storing the same content if it
is very popular, as well as storing a directory of nodes that store that
content.

# Algorithmic implementation #

## Cache node operations ##

This is a non-exhaustive list of the functions/methods the client needs to
implement; any helper functions required to achieve these tasks also would be
needed, and the provided operands are not absolute, as they are somewhat
language-dependent.

- `find_page(page_hash)` Called when a page is requested by a client.
    * Check the hash table for the presence of the hash
    * If the `page_hash` is not present, call `add_page(page_hash)` and
      return the loaded document. (If the request is invalid, return the
      appropriate error document.)
    * Else if the hash is present and the node is local, return the location
      of the requested page in memory.
    * Finally, if the hash is present and the node is remote, return the
      remote location of the page.

- `remove_page(page_hash)` Called by the garbage collector when the cache is
    full and a document that's not present has been requested.
    * Call `notify_nodes(PAGE_REMOVE, page_hash)`.
    * Retrieve the location of the `page_info` struct for the
    `page_hash`.
    * Remove the `page_hash` from the hash table.
    * Remove the `page_info` from the lifetime list.
    * Store the size of the document in a transient variable.
    * Free the document from memory, then free the `page_info` struct.
    * Update the counter of available space.
    * Return the now available space in the table

- `add_page(page_hash)` Called when a page is loaded into the local store on
    a node. Uses a struct called `page_info`, which contains the size of the
    page, its loading time, its time to live, and a pointer to its location in
    memory.
    * Request the page from the server.
    * Check the size of the response; if the size of the response is larger
      than the currently available page space, call
      `garbage_collect(required_size)`.  If the size of the response is 
      larger than the system's maximum available space, skip to replying with
      the content and do not add to the cache. (This breaks the flow of the
      system, ignore this case?)
    * Create a `page_info` struct, then store the document in memory
      and add the memory location of the document to the struct.
    * If the request was invalid, return an invalid request notification.
    * Insert a reference to the `page_info` struct into the hash
      table, set insertion time to this point and initialize time-to-live.
    * Insert the `page_info` struct into the head of the lifetime
      list.
    * Call `notify_nodes(PAGE_ADD, page_hash)`.
    * Return a reference to the `page_info` struct.

- `notify_nodes(state_change, page_hash)` Called when a page is
    added or removed from the hash table.
    * Multicast the node address, the `state_change`, and the `page_hash` to the
      rest of the cache nodes.

- `receive_update(node, state_change, page_hash)` Called when an
    update from another node in the cluster arrives at a given node
    * If the state is `PAGE_ADD` and the page is not already present, add an
      element to the hash table storing that `page_hash` and that node's remote
      information.  If the page is already present, ignore the request.
    * If the state is `PAGE_REMOVE` and the `page_hash` is present in the hash
      table along with the remote information for the same `node`, remove the
      data.  Otherwise, ignore the request.

- `garbage_collect(required_size)` Called when a page needs to
    be added to the cache and there is not enough free space in the node
    currently.
    * While the system's available space is less than the required space, do
      the following:
    * Get the pointer of the last node in the lifetime list.  This can be
      cached. 
    * Move the lifetime pointer up the list one node.
    * Call `remove_page(page_hash)` with the page to be removed.
      (The node itself can contain the `page_hash`, or another route might
      be used.)

- `handle_request(client, requested_page)` Called when a request
    comes in from a client or is forwarded from another node.
    * Generate a `page_hash` from `requested_page`
    * Call `find_page(page_hash)`.
    * If the response was a local node, serve the document from the local
      node. (See below.)
    * If the response was a remote node, by RPC call
    `handle_request(client, requested_page)` on the remote.
    * If the response was an invalid page, serve the error document.
    Serving a document
    * Move the document's `page_info` to the head of the
      lifetime list, and update its time-to-live with the current time plus the
      current time-to-live window.
    * Send document back to client.

## Load balancer operations ##

- `handle_request(client, page)` 
