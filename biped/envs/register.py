from gym.envs.registration import register
import numpy as np

register(
    id="Biped-v0",
    entry_point="envs.biped:BipedEnv",
    max_episode_steps=2000,
    reward_threshold=6000.0,
)


