.PHONEY: images

images:
	docker build -t mesosphere/slave .