### Know bugs which need fixing

 * In the sever during shutdown there is a use after free bug with the socket code. It is in the boost asio context somewhere. While this has never caused a crash for me (I just see asan complain), but it could probably.
 * I think related to the gui code there is a free of a VkImage (or VkImageView I can't remember which) while it is still in use by a command buffer. I *think* it is when the gui textures are updating it something looses all references (It is in my reference counting for the GuiTexture class) and the gpu resources get freed before they should. The code dealing with this is pretty janky and honestly needs a look at anyways, but this is pretty low priority since I am pretty sure it can, (but unlikely does) cause a single texture for one frame on the gui to be coruppted.