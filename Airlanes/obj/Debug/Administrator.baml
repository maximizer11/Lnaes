   M S B A M L   `   `   `  ����C  ?Airlanes, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null#clr-namespace:AirlanesAirlanes  5   	       Airlanes.Administrator   5   	   R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35\ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    5   	   R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089:x,http://schemas.microsoft.com/winfx/2006/xaml     5   	   @d2http://schemas.microsoft.com/expression/blend/2008     5   	   Jmc;http://schemas.openxmlformats.org/markup-compatibility/2006     5   	   (localclr-namespace:Airlanes     5   	     � Title$  Administrator��5   	   $	��450��6   $	��800��6,    � WindowStartupLocation$ CenterScreen=�68    � 
ResizeMode$ NoResize=�6]   .��5	      � .+�5
   
   o� $��25��6   $	��Top=�6   
�������.@�5      �� -   5         ��Name$ 
btnAddUser��5      $��Add user��6)   6<   �� 5      -   5        $ btnExit��5      $
��Exit��6&   65   5      &   System.Windows.Controls.DataGrid  5   
   -   5          Name$ 	dataUsers��5      $��30,75,54,64q�6&      AutoGenerateColumns .     
IsReadOnly . ��5      �� 5       �� 
TargetType)  #System.Windows.Controls.DataGridRow"  6    �� Triggers 5      �� 5      	 �� Binding	 6&   �� *Active+
 �� Value	
 false6A   .a�5      ��  �� Property � �� �� Value
 �  ��6I   5      �� 5      	 6&   �� *	User Role+
 Administrator6D   .a�5      ��  � ��
 ����6P   5      5         Columns 5      0  *System.Windows.Controls.DataGridTextColumn  5      ,  &System.Windows.Controls.DataGridColumn   Header$ ID��6<      
Visibility$ Hidden=�6H   1  +System.Windows.Controls.DataGridBoundColumn   Binding 6%   �� *ID+6H     5      $
 Name��6>    6%   �� *Name+6>     5      $ 	Last Name��6C    6%   �� *	Last Name+6C     5      $	 Age��6=    6%   �� *Age+6=     5       $ 	User Role��6C    6%   �� *	User Role+6C     5!      $ Email Address��6G    6%   �� *Email address+6G     5"      $ Office��6@    6%   �� *Office+6@     5#      $ Active��6@   $ Hidden=�6P    6%   �� *Active+6P   5%      �� 5&   
   -   5         ��Name$ offices��5&       �� DisplayMemberPath$ Title��6$   $
��Left=�6>   $��	77,45,0,0q�6Y   $	��Top=�6l   $	��131��6�   $��19��6�    �� SelectedValuePath$ Title��6�   6�   �� 5'   
   $��Office :��6   $
��Left=�6#   $��	30,40,0,0q�6>   $	��Top=�6Q   �� 5(   
   -   5        $ reset��5(      $��Reset��6    $
��Left=�60   $��
222,45,0,0q�6K   $	��Top=�6_   $��43��6w   6�   �� 5)   
   -   5        $ enable_disable��5)      $��Enable/Disable Login��6)   $
��Left=�6H   $��203,384,0,0q�6c   $	��Top=�6x   $	��123��6�   6�   �� 5*   
   -   5        $ delete��5*      $��Delete��6!   $
��Left=�62   $��
295,45,0,0q�6M   $	��Top=�6a   �� 5+   
   -   5        $ 
changeRole��5+      $��Change Role��6%   $
��Left=�6;   $��
30,384,0,0q�6V   $	��Top=�6j   $	��123��6�   6�   5-      5.      