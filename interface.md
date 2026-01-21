
## Socket Interface to the Executor 

The executor can execute code either via UNIX sockets or TCP sockets depending on configuration. The executor 
communicates via JSON using newlines as delimeters

``` json
 {
      player_id: int,
      game_id: int, 
      user_code: str, 
      inputs_code: list[str]
      test_code: str
 }
```

- player_id: id of the player, results will include the player id 
- game_id: id of the game, results will include the game id
- user_code: code submitted by the user
- inputs_code: a list of the different inputs to test in python code
- test_code: code to test the user submitted code against the inputs 


The executor will then respond back over the socket with the results of the execution 


``` json 
{
      
     status: "OK"
     player_id: int
     game_id: int
     results: [
       { 
           cpu_time_ms: uint, 
           succedeed: bool
           time_limit_exceeded: bool
           tests_failed: bool 
           unknown_error: bool
           stdout: str, 
           stderr: str
       }
     ]
 }
``` 

- status: OK on successful execution (not that test cases failed but everything went according to plan)
          ERROR on error (system errors, etc). See error format below 
- results: Result for each input case sent. Indexed by test case number
    - player_id: player id sent in the original message to execute 
    - game_id: game id sent in the original message to execute 
    - time_limit_exceeded: true if the user program exeeded the set time limit, false otherwise 
    - tests_failed: true if the user failed at lesat 1 test, false otherwise 
    - unknown_error: true if an error occured that is unknown, false otherwise 
    - stdout: captured standard out of the program 
    - stderr: captured standard error of the program 

If a request is malformed or another error occurs, an error response will be sent 
``` json
 { 
    status: "ERROR"
    message: str
 }
``` 



