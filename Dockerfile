##
## Replace "MyGame" with the name of your game, based on the files that Unreal Engine
## built when you made the game server binaries.
##

# Use the UE 5.4 image for packaging
FROM ghcr.io/epicgames/unreal-engine:dev-5.4.1 AS builder

# Set up environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV UE4_ROOT=/home/ue4/UnrealEngine

# Ensure the user 'ue4' exists and create necessary directories as root
USER root

# Create the /app directory and change its ownership
RUN mkdir -p /app && chown -R ue4:ue4 /app

# Set the user to 'ue4' for subsequent commands
USER ue4:ue4

# Set working directory
WORKDIR /tmp/project

# Copy project files to the container
COPY --chown=ue4:ue4 ./ /tmp/project

# Ensure that plugins are included
#COPY --chown=ue4:ue4 ./Plugins/EOSOnlineSubsystem /tmp/project/Plugins/EOSOnlineSubsystem
#COPY --chown=ue4:ue4 ./Plugins/Matchmaking /tmp/project/Plugins/Matchmaking
#COPY --chown=ue4:ue4 ./Plugins/OnlineSubsystemBlueprints /tmp/project/Plugins/OnlineSubsystemBlueprints

# Attempt to package the project as a dedicated server
RUN /home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh \
    BuildCookRun \
    -project=/tmp/project/Server.uproject \
    -noP4 \
    -server \
    -serverplatform=Linux \
    -serverconfig=Development \
    -unrealexe=/home/ue4/UnrealEngine/Engine/Binaries/Linux/UnrealEditor-Cmd \
    -cook \
    -stage \
    -pak \
    -archive \
    -archivedirectory=/tmp/project/BuildOutput \
    -build \
    -compressed

# Stage 2: Use the slim version for copying the packaged files
FROM ghcr.io/epicgames/unreal-engine:dev-slim-5.4.3 AS slimmer

# Set up environment variables
ENV UE4_ROOT=/home/ue4/UnrealEngine

# Copy the build output from the builder stage
COPY --from=builder --chown=ue4:ue4 /tmp/project/BuildOutput/LinuxServer /app

# Stage 3: Use the runtime image for the final execution
FROM ghcr.io/epicgames/unreal-engine:runtime AS runtime

# Create the application directory
#RUN mkdir /app

# Copy the server files from the slimmer stage
COPY --from=slimmer --chown=ue4:ue4 /app /app

# Copy the server startup script to the final image
COPY --from=builder --chown=ue4:ue4 /tmp/project/Releases/LinuxServer/ServerServer.sh /app

# Create a user to run the game server binaries securely
#RUN useradd -m ue4
RUN chown -R ue4:ue4 /app
USER ue4

# Add the game server binaries to the image. Mark the launch script and the main game
# server binary as executable.
#ADD --chown=ue4:ue4 output/LinuxServer /home/ue4
#RUN chmod a+x /home/ue4/ServerServer.sh /home/ue4/Server/Binaries/Linux/app

# Expose the necessary game server ports
EXPOSE 7777/tcp
EXPOSE 7777/udp

# Set the entry point to run the server script when the container starts
ENTRYPOINT ["/app/ServerServer.sh", "-log"]



