
import json 
import sys
import subprocess 

def generate_config(config_path: str): 
    config = {}

    selected = input("Select socket type\n1. Unix\n2. TCP\n")

    socket_config = {}
    config["socket"] = socket_config
    selected = int(selected)

    if selected == 1: 
        socket_config["socket_type"] = "unix"
        path = input("Enter socket path: ")
        socket_config["path"] = path 
    elif selected == 2: 
        socket_config["socket_type"] = "tcp"
        ip = input("Enter IP address: ")
        socket_config["ip_addr"] = ip
        port = input("Enter port: ") 
        socket_config["port"] = int(port)
    else: 
        raise Exception 

    sandbox_cfg = input("Enter sandbox config path: ")
    config["sandbox_config_path"] = sandbox_cfg

    pool_type = input("Select execuiton pool type\n1. Thread Pool (Single Machine)\n2. Emulated\n3. Redis (Multi-Machine)\n")
    pool_type = int(pool_type)

    if pool_type == 1: 
        config["execution_pool_type"] = "thread_pool"
        num_threads = input("Enter number of threads: ")
        config["num_threads"] = int(num_threads)
    elif pool_type == 2: 
        config["execution_pool_type"] = "emulated"
    elif pool_type == 3: 
        config["execution_pool_type"] = "redis"
    else: 
        raise Exception 

    with open(config_path, "w") as f: 
        f.write(json.dumps(config))

    print("Configuration generated")

if __name__ == "__main__": 
    try: 
        generate_config(sys.argv[1] if len(sys.argv) > 1 else "config/config.json")
        answer = input("Restart system service? [y/n]")
        if answer == "y":
            subprocess.run(["systemctl", "restart", "executor"])
    except Exception as e: 
        print("Invalid option")







