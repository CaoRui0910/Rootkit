import numpy as np
from gym import utils
from gym.envs.mujoco import mujoco_env
import os

DEFAULT_CAMERA_CONFIG = {
    "distance": 4.0,
}


class BipedEnv(mujoco_env.MujocoEnv, utils.EzPickle):
    def __init__(
        self,
        xml_file=os.path.join(os.path.join(os.path.dirname(__file__),
                                'asset', "biped.xml")),
    ):
        utils.EzPickle.__init__(**locals())
        mujoco_env.MujocoEnv.__init__(self, xml_file, 5)

    def step(self, action):
        xposbefore = self.get_body_com("robot")[0]
        self.do_simulation(action, self.frame_skip)
        xposafter = self.get_body_com("robot")[0]
        forward_reward = 10.0 * (xposafter - xposbefore) / self.dt

        ctrl_cost = .1 * np.square(action).sum()
        contact_cost = 0.5 * 1e-6 * np.sum(np.square(np.clip(self.sim.data.cfrc_ext, -1, 1)))
        survive_reward = 1.0
        reward = forward_reward + survive_reward - ctrl_cost - contact_cost

        s = self.state_vector()
        angx, angy, angz = self.sim.data.qpos[3:6]
        done = not (np.isfinite(s).all() and (abs(angx) < .5) and (abs(angy) < .5) and (abs(angz) < .5))

        observation = self._get_obs()
        info = {}

        return observation, reward, done, info

    def _get_obs(self):
        position = self.sim.data.qpos.flat.copy()
        velocity = self.sim.data.qvel.flat.copy()

        foot_right_force = self.sim.data.cfrc_ext[5, :3]        # 脚三维力
        foot_left_force = self.sim.data.cfrc_ext[9, :3]
        joint_position = position[6:]                         # 关节位置
        joint_velocity = velocity[6:]                         # 关节速度
        mass_center_position = self.get_body_com("robot")     # 质心位置
        mass_center_angle = self.data.get_body_xquat("robot")  # 质心角度
        position_velocity = self.data.get_body_xvelp("robot")  # 线速度
        angle_velocity = self.data.get_body_xvelr("robot")    # 角速度
        position_acceleration = self.sim.data.cacc[1, :3]    # 线加速度
        angle_acceleration = self.sim.data.cacc[1, -3:]     # 角加速度

        observations = np.concatenate((position, velocity))

        return observations

    def reset_model(self):

        observation = self._get_obs()

        return observation

    def viewer_setup(self):
        for key, value in DEFAULT_CAMERA_CONFIG.items():
            if isinstance(value, np.ndarray):
                getattr(self.viewer.cam, key)[:] = value
            else:
                setattr(self.viewer.cam, key, value)
