.PHONY: install clean install reinstall

package_name = pam_graphql

install: ${package_name}.o
	ld -lcurl -x --shared -o /lib/x86_64-linux-gnu/security/${package_name}.so ${package_name}.o

${package_name}.o: ${package_name}.c
	gcc -fPIC -lcurl -c ${package_name}.c

clean:
	rm -rf ${package_name}.o

uninstall: clean
	rm -rf /lib/x86_64-linux-gnu/security/${package_name}.so 

reinstall: uninstall install
