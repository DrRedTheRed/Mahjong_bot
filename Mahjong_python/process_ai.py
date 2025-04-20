import serial_comm
import time
import kinematics

#做牌捉牌山的各种位置
Initial_yama_position = [[155,185,21],[155,185,8],[135,185,21],[135,185,8],[115,185,21],[115,185,8],[95,185,21],[95,185,8],[75,185,21],[75,185,8],[55,185,21],[55,185,8],[35,185,21]]
#打牌捉牌山的各种位置
Discard_yama_position = [[35,185,8],[-5,185,8],[-45,185,8],[-85,185,8]] 
#放牌区位置
Destination_position = [[-180,80,0], [100,115,0],[100,115,0],[88,115,0],[40,115,0],[20,115,0],[0,115,0],[-20,115,0],[-40,115,0],[-60,115,0],[-80,115,0],[-100,115,0],[-120,115,0],[-140,115,0]] #第一个位置实际上是self.temporary_position

class MahjongArmStateMachine_initial:
    def __init__(self):
        # 初始状态
        self.state = 'Idle'

        self.tile_position = None
        self.camera_position = [180,80,0] #相机位置相对固定，可以写死
        self.destination = None
        self.original_position = [100, 100, 50]  # 机械臂初始位置为 [100, 100, 50]
        self.arm_position = self.original_position
        self.temporary_position = [-180,80,0]
        self.discard_position = [10,280,80]
        self.offset = [0,0,40] # 夹爪下降的偏移量
        self.goal = 'Camera' # 状态机的判别条件

    def set_task(self, tile_position, destination):
        """设置抓取任务"""
        self.tile_position = tile_position
        self.destination = destination
        self.state = 'MoveToTile'
        self.goal = 'Camera'

    def set_task_discard(self, tile_position):
        """设置抓取任务"""
        self.tile_position = tile_position
        self.state = 'MoveToTile'
        self.goal = 'Camera'

    def set_task_discard1(self, destination):
        """设置抓取任务"""
        self.destination = destination


    def move_to(self, start_position, target_position, trajectory_type):
        """移动到目标位置"""
        print(f"Moving from {start_position} to {target_position} in {trajectory_type}...",flush=True)
        kinematics.move_as_tajectory(start_position, target_position, trajectory_type)  # 发送移动指令 
        time.sleep(2) # 等待移动完成
        return True  # 假设移动总是成功

    def control_gripper(self, action):
        """控制夹爪"""
        
        print("Lowering gripper...")
        lower = [c - o for c, o in zip(self.arm_position, self.offset)]
        kinematics.move_as_tajectory(self.arm_position, lower, 'linear')  # 发送夹爪下降指令
        time.sleep(1)  # 等待夹爪下降完成

        if action == 'close':
            print("Closing gripper...",flush=True)
            serial_comm.send_to_serial('#005P2200T0500!\r')  # 发送夹爪闭合指令
            time.sleep(1)  # 等待夹爪闭合完成         
        elif action == 'open':
            print("Opening gripper...",flush=True)
            serial_comm.send_to_serial('#005P1500T0500!\r') # 发送夹爪张开指令
            time.sleep(1)  # 等待夹爪张开完成
              
        print("Gripper ascending...",flush=True)
        kinematics.move_as_tajectory(lower, self.arm_position, 'linear')  # 发送夹爪上升指令
        time.sleep(1)

        return True  # 假设夹爪动作总是成功

    def run_drawAtile(self):
        """状态机运行"""
        while self.state != 'Done':
            if self.state == 'Idle':
                print("Idle state, waiting for task...",flush=True)
            
            elif self.state == 'MoveToTile':
                if self.move_to(self.original_position,self.tile_position, 'parabola'):
                    self.arm_position = self.tile_position
                    self.state = 'GrabTile'

            elif self.state == 'GrabTile':
                if self.control_gripper('close'):
                    if self.goal == 'Camera':
                        self.state = 'MoveToCamera'
                    elif self.goal == 'Destination':
                        self.state = 'MoveToDestination'
                        self.goal = 'Done'

            elif self.state == 'MoveToCamera':
                if self.move_to(self.tile_position,self.camera_position, 'parabola'):
                    self.arm_position = self.camera_position
                    self.state = 'ReleaseTile'

            elif self.state == 'ReleaseTile':
                if self.control_gripper('open'):
                    if self.goal == 'Camera':
                        self.state = 'GrabTile'
                        self.goal = 'Destination'
                        break #状态机需要之后继续运行
                    elif self.goal == 'Done':
                        self.state = 'Done'

            elif self.state == 'MoveToDestination':
                if self.move_to(self.camera_position,self.destination, 'parabola'):
                    self.arm_position = self.destination
                    self.state = 'ReleaseTile'


        if self.state == 'Done' and self.move_to(self.destination,self.original_position,'parabola'):
            self.arm_position = self.original_position
            print("Task completed!",flush=True)

        print("State machine finished.",flush=True)


    def run_playAtile(self):
        """状态机运行"""
        while self.state != 'Done':
            if self.state == 'Idle':
                print("Idle state, waiting for task...",flush=True)
            
            elif self.state == 'MoveToTile':
                if self.move_to(self.original_position,self.tile_position, 'parabola'):
                    self.arm_position = self.tile_position
                    self.goal = 'Camera'
                    self.state = 'GrabTile'
            
            elif self.state == 'MoveToTemporary_Area':
                if self.goal == 'Temporary_Area':
                    if self.move_to(self.camera_position,self.temporary_position, 'parabola'):
                        self.arm_position = self.temporary_position
                        self.state = 'ReleaseTile'
                elif self.goal =='Destination':
                    if self.move_to(self.discard_position,self.temporary_position, 'parabola'):
                        self.arm_position = self.temporary_position
                        self.state = 'GrabTile'

            
            elif self.state == 'GrabTile':
                if self.control_gripper('close'):
                    if self.goal == 'Camera':
                        self.state = 'MoveToCamera'
                    elif self.goal == 'Temporary_Area':
                        self.state = 'MoveToTemporary_Area'
                    elif self.goal == 'Destination':
                        self.state = 'MoveToDestination'
                    elif self.goal == 'Discard':
                        self.state = 'MoveToDiscardArea'
                        # self.goal = 'Done'


            elif self.state == 'MoveToDiscardArea':
                if self.move_to(self.destination,self.discard_position, 'parabola'): 
                    self.arm_position = self.discard_position
                    self.state = 'ReleaseTile'

            elif self.state == 'MoveToCamera':
                if self.move_to(self.tile_position,self.camera_position, 'parabola'):
                    self.arm_position = self.camera_position
                    self.state = 'ReleaseTile'

            elif self.state == 'ReleaseTile':
                if self.control_gripper('open'):
                    if self.goal == 'Camera':
                        self.state = 'GrabTile'
                        self.goal = 'Temporary_Area'
                        break
                    elif self.goal == 'Temporary_Area':
                        self.state = 'MoveToDestination'
                        self.goal = 'Discard'
                    elif self.goal == 'Discard':
                        self.state = 'MoveToTemporary_Area'
                        self.goal = 'Destination'
                    elif self.goal == 'Destination':
                        self.state = 'Done'
                    elif self.goal == 'Done':
                        self.state = 'Done'

            elif self.state == 'MoveToDestination':
                if self.move_to(self.temporary_position,self.destination, 'parabola'):
                    self.arm_position = self.destination
                    if self.goal == 'Destination':
                        self.state = 'ReleaseTile'
                    elif self.goal == 'Discard':
                        self.state = 'GrabTile'
                        


        if self.state == 'Done' and self.move_to(self.destination,self.original_position,'parabola'):
            self.arm_position = self.original_position
            print("Task completed!")

        print("State machine finished.")



#动作组分解
def process_ai_data_initial(number):
    tile_position = Initial_yama_position[number]
    destination = Destination_position[number]
    return tile_position, destination

def process_ai_data_discard(number,Mahjong_Round):
    
    tile_position = Discard_yama_position[Mahjong_Round]
    destination = Destination_position[number]
    return tile_position, destination

# 测试状态机
if __name__ == "__main__":
    # 定义麻将和目标位置
    tile_position = Initial_yama_position[0]  # 假设第n个麻将的位置
    destination = Destination_position[0]   # 假设放置的目标位置
    discard_position = Discard_yama_position #假设打牌的位置

    # 创建状态机实例
    arm_state_machine = MahjongArmStateMachine_initial()

    # 设置任务
    arm_state_machine.set_task(tile_position, destination)
    #在mahjong_ai函数中还要加：tile_position, destination = process_ai_data(number)

    # 启动状态机
    arm_state_machine.run_playAtile()

    arm_state_machine.run_playAtile()