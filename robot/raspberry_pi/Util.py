import threading

class Util():
    @staticmethod
    def get_thread_info():
        print(f"Threads alive count: {threading.active_count()}\n"
              f"Thread current: {threading.current_thread()}")
        
