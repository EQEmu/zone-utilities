VERSION ?= 0.0.6
NAME := zone-utilies
REPOSITORY := xackery

docker-build:
	docker build -t $(NAME) .

docker-push: docker-build
	docker tag $(NAME) $(REPOSITORY)/$(NAME):$(VERSION)
	docker push $(REPOSITORY)/$(NAME):$(VERSION)
	docker tag $(NAME) $(REPOSITORY)/$(NAME):latest
	docker push $(REPOSITORY)/$(NAME):latest