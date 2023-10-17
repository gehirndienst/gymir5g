import traceback

from core.args import get_argparser, GymirArgs
from core.drl_manager import GymirDrlManager

""" 
    console run module with the __main__ endpoint
"""

if __name__ == "__main__":
    # get the args
    args = GymirArgs.from_namespace(get_argparser().parse_args())

    # initialize the manager
    drl_manager = GymirDrlManager(args)

    # training or evaluation?
    mode = args.mode
    try:
        if mode == "train":
            drl_manager.learn()
        else:
            drl_manager.eval()
    except KeyboardInterrupt:
        print("The main execution was interrupted externally from the keyboard")
    except BaseException:
        print(traceback.print_exc())
    finally:
        drl_manager.env.close()
