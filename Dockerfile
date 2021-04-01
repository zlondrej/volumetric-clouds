FROM ubuntu

RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" apt-get install tzdata
RUN apt-get install make g++ libsdl2-dev libglm-dev libgl-dev libglew-dev --yes