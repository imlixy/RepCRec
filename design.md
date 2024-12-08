# Design Document

## Diagram

The system uses a Master-Slave architecture with a single `TransactionManager` as the master node and 10 `DataManager` nodes as slaves.

The `TransactionManager` receives inputs (transactions), processes commands, and coordinates with `DataManager` nodes to execute operations. Each `DataManager` manages local data variables and responds to requests from the `TransactionManager`.

Below is the architecture diagram:

<img src="D:\Typora_result\Typora_image\image-20241208064809942.png" alt="image-20241208064809942" style="zoom:80%;" />

## Module Design

### 1. Transaction Manager

The `TransactionManager` is the central controller of the system. It manages the lifecycle of transactions, coordinates data access across `DataManager` nodes, and `SerialzationGraph`.

**Responsibilities**:

- Manages the lifecycle of transactions, including `begin`, `read`, `write`, and `end`.
- Coordinates with `DataManager` nodes to perform site-level data operations (e.g., variable read and writes).
- Tracks and handles site failures and recoveries, ensuring proper transaction blocking and resumption.
- Maintains an active list of transactions and their states (`active`, `blocked`, `committed`, `aborted`).
- Interfaces with the `SerializationGraph` for dependency tracking and cycle detection, ensuring serializability.

### 2. Data Manager

Each site corresponds to a `DataManager` instance, which maintains local data, tracks site status, and handles operations related to variable reads, writes, and recoveries.

**Responsibilities**:

- Variable Management:

  - Manages the committed values of variables along with their version histories.
  - Tracks uncommitted writes in a local cache for transactions.
  
- Site State Management:

  - Maintains the status of the site (`available`, `failed`, or `recovering`).
  - Handles site failures by marking the site as unavailable and clearing uncommitted writes.
  - Manages recovery processes, including restoring the accessibility of replicated variables.

- Transaction Interaction:

  - Responds to read and write requests from the `TransactionManager`.
  - Provides variable values based on version consistency checks.

### 3. Serialization Graph

The `SerializationGraph` module models the dependency graph for transactions. It tracks relationships between transactions using a directed graph, where nodes represent transactions and edges represent dependencies (e.g., read-write or write-write conflicts). This module plays a critical role in detecting cycles to identify potential conflicts.

**Responsibilities**:

- Tracks transactions and their dependencies as nodes and edges in the graph.
- Provides functionality to add and remove transactions and dependencies dynamically.
- Detects cycles in the graph to ensure serializability.



## Data Structures

The data structure of each module is as follows, and there may be subsequent modifications.

### Transaction Manager

<img src="D:\Typora_result\Typora_image\image-20241208071802244.png" alt="image-20241208071802244" style="zoom:67%;" />

### Data Manager

<img src="D:\Typora_result\Typora_image\image-20241208072204215.png" alt="image-20241208072204215" style="zoom:67%;" />

### Serialization Graph

<img src="D:\Typora_result\Typora_image\image-20241208072421737.png" alt="image-20241208072421737" style="zoom:55%;" />

## Main Function

Here is the main functionality of the functions, along with their inputs and outputs.

### Transaction Manager

1. `inputHandle(const string& inputs)`
   - **Function**: Parses an input string command, recognizes its type, and delegates the operation to the appropriate transaction function.
   - **Input**: A single line string command (e.g., `begin(T1)`, `R(T1, x3)`).
   - **Output**: Executes the corresponding operation or logs errors for invalid commands.

2. `beginTransaction(tran_id tranID)`

   - **Function**: Initializes a new transaction with a unique transaction ID.

   - **Input**:
     - `tranID`: A unique identifier for the new transaction.
   
    - **Output**:
       - None.
   
   - **Detail**:
      - Sets the transaction status to `active` and logs it in `transList`.
      - Adds the transaction as node to the serialization graph.

3. `readTransaction(tran_id tranID, var_id varID)`

   - **Function**: Processes a read request for a variable by a transaction.

   - **Input**:
       - `tranID`: Transaction ID.
       - `varID`: Variable ID to be read.

   - **Output**:
       - The value of the variable (printed to the console).

   - **Details**:
       - Checks if the transaction exists and is `active`.
       - For *replicated variables*:
         - Tries all available sites to read the most recent committed version.
       - For *non-replicated variables*:
         - Reads from the designated site.
       - Updates the serialization graph to reflect `RW` dependencies.
       - Handles waiting logic if the read cannot proceed due to site failures.

4. `writeTransaction(tran_id tranID, var_id varID, int value)`

   - **Function**: Processes a write request for a variable by a transaction.

   - **Input**:
       - `tranID`: Transaction ID.
       - `varID`: Variable ID to be written.
       - `value`: The value to write to the variable.

   - **Output**:
       - None.

   - **Details**
       - Checks if the transaction exists and is `active`.
       - Updates the serialization graph for `WW` and `RW` conflicts.
       - Writes the value to the target site(s), depending on whether the variable is replicated or non-replicated. (local write)
       - Logs any write failures.

5. `endTransaction(tran_id tranID)`

   - **Function**: Attempts to commit or abort the transaction based on conditions.

   - **Input**:
       - `tranID`: Transaction ID.

   - **Output**:
       - Logs the result (`committed` or `aborted`).

   - **Details**:
       - Aborts immediately if the transaction is `aborted` or `blocked`.
       - Checks:
         - Write consistency across all sites for replicated variables.
         - The presence of cycles in the serialization graph.
         - `WAW` conflicts, applying the first-committer-wins rule.
       - Commits the transaction if all checks pass and propagates writes to sites.

6. `fail(site_id siteID)`

   - **Function**: Marks a site as unavailable and clears its cache of uncommitted writes.

   - **Input**:
       - `siteID`: The ID of the site to fail.

   - **Output**:
       - None.

   - **Details** :
       - Updates the site's availability status.
       - Clears all local writes on the site.

7. `recover(site_id siteID)`

   - **Function**: Recovers a previously failed site and processes blocked transactions.

   - **Input** :
       - `siteID`: The ID of the site to recover.

   - **Output** :
       - None.

   - **Details**:
       - Marks the site as available.
       - Checks all blocked transactions for variables that can now be read from the recovered site.
       - Updates the serialization graph and transaction statuses accordingly.

8. `dump()`

   - **Function**: Prints the latest committed values of all variables on each site, grouped by site ID.

   - **Input**:
       - None.

   - **Output**:
       - A list of all variables and their values at each site.

   - **Details**:
       - Sorts variables by ID within each site for clarity.

9. `queryState()`

   - **Function**: Debugging utility that prints the state of the `TransactionManager` and each `DataManager`.

   - **Input**: 
       - None

   - **Output**:
       - Detailed information about the transaction statuses and site-level cached writes.

10. `abortTransaction(const tran_id tranID)`

   - **Function**: Handles the abortion of a transaction by cleaning up its cached writes, updating its status, and removing it from the serialization graph.
     
  > This function is private in Transaction Manager class, can be called by `read`, `end`.

   - **Input**:
     
- `tranID`: The ID of the transaction to be aborted.
  
   - **Output**:
   
- None (effects are directly on the transaction and site data).
  
   - **Details**:
     - Iterates over all `DataManager` instances (sites) to:
        - Call `abortWrite` on the corresponding site to discard any cached writes for the transaction.
     - Updates the transaction's status to `aborted`.
     - Removes the transaction from `transList` and the transaction node from the `tranGraph`, clearing all dependencies.
     - Prints a log message indicating the transaction's abortion.

11. `commitTransaction(const tran_id tranID)`

   - **Function**: Handles the successful commit of a transaction by making all its cached writes permanent and updating its status.
     
   > This function is private in Transaction Manager class, can be called bu `end`.

   - **Input**:
     
- `tranID`: The ID of the transaction to be committed.
  
   - **Output**:
   
- None (effects are directly on the transaction and site data).

   - **Details**:
     1. Iterates over the transaction's `write` set to commit each variable:
        - For *replicated variables*:
          - Writes to all available sites that host the variable.
        - For *non-replicated variables*:
          - Writes only to the target site, provided the site is available.

     2. Calls `commitWrite` on the corresponding site to:
        - Update the site's version history for the variable.
        - Clear the cached write for the transaction.
     3. Updates the transaction's status to `committed`, and prints a log message indicating the transaction's successful commit.

### Data Manager

1. `pair<bool, int> read(const var_id varID, const double startTime)`

   - **Function**: Reads the committed value of a variable as of the transaction's start time. 
    > This function can be called by `read` and `recover` from Transaction Manager.
   - **Input**:
     - `varID`: The variable to be read.
     - `startTime`: The start time of the transaction requesting the read.
   - **Output**:
     - A pair where:
       - `bool`: Indicates whether the read was successful.
       - `int`: The value of the variable or an error code.
   - **Details**:
     - Looks up the version history to find the value committed before `startTime`.
     - For replicated variables, ensures a committed write exists after the last failure if applicable.
     - Handles edge cases like site failures and unavailable variables.

2. `bool write(const tran_id tranID, const var_id varID, const int value)`

   - **Function**: Writes a value for a variable to the site's local cache as part of a transaction. 
     
   > This function can be called by `write` from Transaction Manager.
   
   - **Input**:
     - `tranID`: The ID of the transaction performing the write.
     - `varID`: The variable to be written.
     - `value`: The new value for the variable.

   - **Output**:
     
      - A boolean indicating whether the write was successful.
     
   - **Details**:
     - Ensures the site is available and the variable exists.
     - Stores the write in a cache specific to the transaction.

3. `void commitWrite(const tran_id tranID, const var_id varID, const int value)`

   - **Function**: Commits a cached write for a transaction, making the changes permanent in the version history. 
     
   > This function can be called by `commitTransaction` from Transaction Manager.

   - **Input** :
     - `tranID`: The ID of the transaction committing the write.
     - `varID`: The variable being committed.
     - `value`: The value to commit.

   - **Output** : None.

   - **Details** :
     - Updates the variable's current value and version history.
     - Clears the cached write for the transaction.

4. `void abortWrite(const tran_id tranID)`

   - **Function**: Clears all cached writes for a transaction.
      
   > The function can be called by `abortTransaction` from Transaction Manager.
      
   - **Input** :
      
     - `tranID`: The ID of the transaction to abort.
     
   - **Output** : None.

   - **Details** :
     
     - Removes all entries for the transaction from the cache.

5. `void setAvailable(bool flag)`

   - **Function**: Sets the site's availability and updates failure/recovery times.
      
   > This function can be called by `fail` and `recover` from Transaction Manager.
      
   - **Input**:
      
     - `flag`: A boolean indicating whether the site is available.
     
   - **Output** : None.

6. `void clearCache()`

   - **Function**: Clears all cached writes for the site.
     
     > This function can be called by `recover` from Transaction Manager.
     
   - **Input** : None.

   - **Output** : None.

### Serialization Graph

1. `addTran(const tran_id tranID)`

   - **Function**: Adds a new transaction (node) to the graph.
     
      > This function can be called by `begin` from Transaction Manager.
      
   - **Input** :
      
     - `tranID`: A unique identifier for the transaction to be added.
     
   - **Output** :
      
     - None.
     
   - **Details**:
     - Initializes an empty adjacency list for the new transaction.
     - Ensures no duplicate nodes are added.

2. `addDependency(const tran_id u, const tran_id v, EdgeType type)`

   - **Function**: Adds a directed edge between two transactions, representing a dependency.
     
   > This function can be called by `read`, `write`, `recover` from Transaction Manager.
   
   - **Input** :
     - `u`: The source transaction ID.
     - `v`: The target transaction ID.
     - `type`: The type of edge (`RW`, `WW`).

   - **Output**:
     
   - None.
     
   - **Details** :
     - Updates the adjacency list of the source transaction.
     - Validates that both transactions exist in the graph.

3. `removeTran(const tran_id tranID)`

   - **Function**: Removes a transaction (node) and all associated edges from the graph.
     
      > This function can be called by `abortTransaction` from Transaction Manager.
     
   - **Input** :
      
     - `tranID`: The ID of the transaction to remove.
     
   - **Output** :
     
     - None.

4. `bool hasCycle(const tran_id tranID, const unordered_map<tran_id, Status>& statusList) const`

   - **Function**: Checks if a cycle exists in the graph that includes the specified transaction and meets the status criteria.
     
     > This function can be called by `end` from Transaction Manager.
     
   - **Input** :
     - `tranID`: The target transaction to check for cycles.
     - `statusList`: A mapping of transaction IDs to their statuses (`commit`, `running`, etc.).

   - **Output** :
     
   - `true` if a valid cycle is found, `false` otherwise.
     
   - **Details** :
     - Calls `detectCycle` recursively to traverse the graph.
     - Verifies that cycles include the target transaction and meet the specified conditions.

5. `bool detectCycle(...) const`

   - **Function**: Recursively detects cycles in the graph starting from a given node.
     
     > This function is private in Serialization Graph class, can be called by `hasCycle`.
     
   - **Input** :
     - `tranID`: The target transaction for cycle validation.
     - `node`: The current node being visited.
     - `flag`: A set of visited nodes to avoid reprocessing.
     - `stack`: A set representing the current recursion stack.
     - `parent`: A map tracking the parent of each visited node.
     - `statusList`: A mapping of transaction IDs to their statuses.

   - **Output** :
      
     - `true` if a cycle is detected, `false` otherwise.
     
   - **Details**:
     
     - Traverses the graph depth-first.
     - Tracks the recursion path and validates any detected cycles using `isValidCycle`.

6. `bool isValidCycle(...) const`

   - **Function**: Validates whether a detected cycle includes the target transaction and meets the status criteria.
     
     > This function is private in Serialization Graph class, can be called be `detectCycle`
     
   - **Input** :
     - `tranID`: The target transaction ID.
     - `start`: The start node of the cycle.
     - `end`: The end node of the cycle.
     - `parent`: A map of parent nodes for reconstructing the cycle path.
     - `statusList`: A mapping of transaction IDs to their statuses.
   
   - **Output** :
      
     - `true` if the cycle is valid, `false` otherwise.
     
   - **Details**:
     - Reconstructs the cycle path from the parent map.
     - Ensures all nodes in the cycle (except the target transaction) meet the status requirement (`commit`).