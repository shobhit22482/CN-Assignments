all:
	gcc multi_thread_server.c -o multi_thread_server -pthread
	gcc single_thread_server.c -o single_thread_server
	gcc server_select.c -o server_select
	gcc multi_thread_client.c -o multi_thread_client -pthread
	gcc single_thread_client.c -o single_thread_client -pthread
	gcc client_select.c -o client_select -pthread

clean:
	rm -f multi_thread_server single_thread_server multi_thread_client single_thread_client server_select client_select
