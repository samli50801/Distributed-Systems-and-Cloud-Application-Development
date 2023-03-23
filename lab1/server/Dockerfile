FROM python:3.8

#ADD . /server
#WORKDIR /server

# Copy host file to container
ADD . .
RUN mkdir -p data

# Dependency
RUN pip install Flask
ENV FLASK_APP=server.py
ENV FLASK_RUN_HOST=0.0.0.0

# Run server.py
CMD flask run


