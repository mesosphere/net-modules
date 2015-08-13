.PHONEY: images

images:
	docker build --no-cache -t mesosphere/slave .
